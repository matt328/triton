#include "GltfImporter.hpp"

#include "GltfHelpers.hpp"

namespace al::gltf {
   GltfImporter::GltfImporter() {
      // TODO: Actual Image Loader
      auto image_loader = [](tinygltf::Image*,
                             const int,
                             std::string*,
                             std::string*,
                             int,
                             int,
                             const unsigned char*,
                             int,
                             void*) { return true; };
      m_loader.SetImageLoader(image_loader, nullptr);
   }

   bool GltfImporter::Load(const char* filename) {
      auto success = false;
      std::string errors;
      std::string warnings;

      // Finds file extension.
      const auto separator = std::strrchr(filename, '.');
      const auto ext = separator != nullptr ? separator + 1 : "";

      // Tries to guess whether the input is a gltf json or a glb binary based on
      // the file extension
      if (std::strcmp(ext, "glb") == 0) {
         success = m_loader.LoadBinaryFromFile(&m_model, &errors, &warnings, filename);
      } else {
         if (std::strcmp(ext, "gltf") != 0) {
            Log::error << "Unknown file extension '" << ext << "', assuming a JSON-formatted gltf."
                       << std::endl;
         }
         success = m_loader.LoadASCIIFromFile(&m_model, &errors, &warnings, filename);
      }

      // Prints any errors or warnings emitted by the loader
      if (!warnings.empty()) {
         Log::warn << "glTF parsing warnings: " << warnings << std::endl;
      }

      if (!errors.empty()) {
         Log::error << "glTF parsing errors: " << errors << std::endl;
      }

      if (success) {
         Log::info << "glTF parsed successfully." << std::endl;
      }

      if (success) {
         success &= fixupNames(m_model.scenes, "Scene", "scene_");
         success &= fixupNames(m_model.nodes, "Node", "node_");
         success &= fixupNames(m_model.animations, "Animation", "animation_");
      }

      return success;
   }

   void GltfImporter::FindSkinRootJointIndices(const ozz::vector<tinygltf::Skin>& skins,
                                               ozz::vector<int>& roots) {
      static constexpr int no_parent = -1;
      static constexpr int visited = -2;

      ozz::vector<int> parents(m_model.nodes.size(), no_parent);

      for (int node = 0; node < static_cast<int>(m_model.nodes.size()); node++) {
         for (int child : m_model.nodes[node].children) {
            parents[child] = node;
         }
      }

      for (const tinygltf::Skin& skin : skins) {
         if (skin.joints.empty()) {
            continue;
         }

         if (skin.skeleton != -1) {
            parents[skin.skeleton] = visited;
            roots.push_back(skin.skeleton);
            continue;
         }

         int root = skin.joints[0];
         while (root != visited && parents[root] != no_parent) {
            root = parents[root];
         }
         if (root != visited) {
            roots.push_back(root);
         }
      }
   }

   bool GltfImporter::Import(ozz::animation::offline::RawSkeleton* skeleton,
                             const NodeType& types) {
      (void)types;

      if (m_model.scenes.empty()) {
         Log::error << "No scenes found." << std::endl;
         return false;
      }

      // If no default scene has been set then take the first one spec does not
      // disallow gltfs without a default scene but it makes more sense to keep
      // going instead of throwing an error here
      int defaultScene = m_model.defaultScene;
      if (defaultScene == -1) {
         defaultScene = 0;
      }

      tinygltf::Scene& scene = m_model.scenes[defaultScene];
      Log::debug << "Importing from default scene #" << defaultScene << " with name \""
                 << scene.name << "\"." << std::endl;

      if (scene.nodes.empty()) {
         Log::error << "Scene has no node." << std::endl;
         return false;
      }

      // Get all the skins belonging to this scene
      ozz::vector<int> roots;
      ozz::vector<tinygltf::Skin> skins = GetSkinsForScene(scene);
      if (skins.empty()) {
         Log::info << "No skin exists in the scene, the whole scene graph "
                      "will be considered as a skeleton."
                   << std::endl;
         // Uses all scene nodes.
         for (auto& node : scene.nodes) {
            roots.push_back(node);
         }
      } else {
         if (skins.size() > 1) {
            Log::info << "Multiple skins exist in the scene, they will all "
                         "be exported to a single skeleton."
                      << std::endl;
         }

         // Uses all skins roots.
         FindSkinRootJointIndices(skins, roots);
      }

      // Remove nodes listed multiple times.
      std::sort(roots.begin(), roots.end());
      roots.erase(std::unique(roots.begin(), roots.end()), roots.end());

      // Traverses the scene graph and record all joints starting from the roots.
      skeleton->roots.resize(roots.size());
      for (size_t i = 0; i < roots.size(); ++i) {
         const tinygltf::Node& root_node = m_model.nodes[roots[i]];
         ozz::animation::offline::RawSkeleton::Joint& root_joint = skeleton->roots[i];
         if (!ImportNode(root_node, &root_joint)) {
            return false;
         }
      }

      if (!skeleton->Validate()) {
         Log::error << "Output skeleton failed validation. This is likely an "
                       "implementation issue."
                    << std::endl;
         return false;
      }

      return true;
   }

   bool GltfImporter::Import(const char* animationName,
                             const ozz::animation::Skeleton& skeleton,
                             float samplingRate,
                             ozz::animation::offline::RawAnimation* animation) {
      if (samplingRate == 0.0f) {
         samplingRate = DefaultSamplingRate;

         static bool samplingRateWarn = false;
         if (!samplingRateWarn) {
            Log::debug << "The animation sampling rate is set to 0 "
                          "(automatic) but glTF does not carry scene frame "
                          "rate information. Assuming a sampling rate of "
                       << samplingRate << "hz." << std::endl;

            samplingRateWarn = true;
         }
      }

      // Find the corresponding gltf animation
      std::vector<tinygltf::Animation>::const_iterator gltf_animation =
          std::find_if(begin(m_model.animations),
                       end(m_model.animations),
                       [animationName](const tinygltf::Animation& _animation) {
                          return _animation.name == animationName;
                       });
      assert(gltf_animation != end(m_model.animations));

      animation->name = gltf_animation->name.c_str();

      // Animation duration is determined during sampling from the duration of the
      // longest channel
      animation->duration = 0.0f;

      const int num_joints = skeleton.num_joints();
      animation->tracks.resize(num_joints);

      // gltf stores animations by splitting them in channels
      // where each channel targets a node's property i.e. translation, rotation
      // or scale. ozz expects animations to be stored per joint so we create a
      // map where we record the associated channels for each joint
      ozz::cstring_map<std::vector<const tinygltf::AnimationChannel*>> channels_per_joint;

      for (const tinygltf::AnimationChannel& channel : gltf_animation->channels) {
         // Reject if no node is targetted.
         if (channel.target_node == -1) {
            continue;
         }

         // Reject if path isn't about skeleton animation.
         bool valid_target = false;
         for (const char* path : {"translation", "rotation", "scale"}) {
            valid_target |= channel.target_path == path;
         }
         if (!valid_target) {
            continue;
         }

         const tinygltf::Node& target_node = m_model.nodes[channel.target_node];
         channels_per_joint[target_node.name.c_str()].push_back(&channel);
      }

      // For each joint get all its associated channels, sample them and record
      // the samples in the joint track
      const ozz::span<const char* const> joint_names = skeleton.joint_names();
      for (int i = 0; i < num_joints; i++) {
         auto& channels = channels_per_joint[joint_names[i]];
         auto& track = animation->tracks[i];

         for (auto& channel : channels) {
            auto& sampler = gltf_animation->samplers[channel->sampler];
            if (!SampleAnimationChannel(m_model,
                                        sampler,
                                        channel->target_path,
                                        samplingRate,
                                        &animation->duration,
                                        &track)) {
               return false;
            }
         }

         const tinygltf::Node* node = FindNodeByName(joint_names[i]);
         assert(node != nullptr);

         // Pads the rest pose transform for any joints which do not have an
         // associated channel for this animation
         if (track.translations.empty()) {
            track.translations.push_back(createTranslationRestPoseKey(*node));
         }
         if (track.rotations.empty()) {
            track.rotations.push_back(createRotationRestPoseKey(*node));
         }
         if (track.scales.empty()) {
            track.scales.push_back(createScaleRestPoseKey(*node));
         }
      }

      Log::debug << "Processed animation '" << animation->name
                 << "' (tracks: " << animation->tracks.size()
                 << ", duration: " << animation->duration << "s)." << std::endl;

      if (!animation->Validate()) {
         Log::error << "Animation '" << animation->name << "' failed validation." << std::endl;
         return false;
      }

      return true;
   }

   bool GltfImporter::ImportNode(const tinygltf::Node& node,
                                 ozz::animation::offline::RawSkeleton::Joint* joint) {
      // Names joint.
      joint->name = node.name.c_str();

      // Fills transform.
      if (!createNodeTransform(node, &joint->transform)) {
         return false;
      }

      // Allocates all children at once.
      joint->children.resize(node.children.size());

      // Fills each child information.
      for (size_t i = 0; i < node.children.size(); ++i) {
         const tinygltf::Node& child_node = m_model.nodes[node.children[i]];
         ozz::animation::offline::RawSkeleton::Joint& child_joint = joint->children[i];

         if (!ImportNode(child_node, &child_joint)) {
            return false;
         }
      }

      return true;
   }

   GltfImporter::AnimationNames GltfImporter::GetAnimationNames() {
      AnimationNames animNames;
      for (auto& animation : m_model.animations) {
         assert(animation.name.length() != 0);
         animNames.emplace_back(animation.name.c_str());
      }

      return animNames;
   }

   bool GltfImporter::SampleAnimationChannel(
       const tinygltf::Model& model,
       const tinygltf::AnimationSampler& sampler,
       const std::string& targetPath,
       float samplingRate,
       float* _duration,
       ozz::animation::offline::RawAnimation::JointTrack* track) {
      // Validate interpolation type.
      if (sampler.interpolation.empty()) {
         Log::error << "Invalid sampler interpolation." << std::endl;
         return false;
      }

      auto& input = m_model.accessors[sampler.input];
      assert(input.maxValues.size() == 1);

      // The max[0] property of the input accessor is the animation duration
      // this is required to be present by the spec:
      // "Animation Sampler's input accessor must have min and max properties
      // defined."
      const float duration = static_cast<float>(input.maxValues[0]);

      // If this channel's duration is larger than the animation's duration
      // then increase the animation duration to match.
      if (duration > *_duration) {
         *_duration = duration;
      }

      assert(input.type == TINYGLTF_TYPE_SCALAR);
      auto& _output = m_model.accessors[sampler.output];
      assert(_output.type == TINYGLTF_TYPE_VEC3 || _output.type == TINYGLTF_TYPE_VEC4);

      const ozz::span<const float> timestamps = bufferView<float>(model, input);
      if (timestamps.empty()) {
         return true;
      }

      // Builds keyframes.
      bool valid = false;
      if (targetPath == "translation") {
         valid = sampleChannel(m_model,
                               sampler.interpolation,
                               _output,
                               timestamps,
                               samplingRate,
                               duration,
                               &track->translations);
      } else if (targetPath == "rotation") {
         valid = sampleChannel(m_model,
                               sampler.interpolation,
                               _output,
                               timestamps,
                               samplingRate,
                               duration,
                               &track->rotations);
         if (valid) {
            // Normalize quaternions.
            for (auto& key : track->rotations) {
               key.value = ozz::math::Normalize(key.value);
            }
         }
      } else if (targetPath == "scale") {
         valid = sampleChannel(m_model,
                               sampler.interpolation,
                               _output,
                               timestamps,
                               samplingRate,
                               duration,
                               &track->scales);
      } else {
         assert(false && "Invalid target path");
      }

      return valid;
   }

   [[nodiscard]] ozz::vector<tinygltf::Skin> GltfImporter::GetSkinsForScene(
       const tinygltf::Scene& scene) const {
      ozz::set<int> open;
      ozz::set<int> found;

      for (int nodeIndex : scene.nodes) {
         open.insert(nodeIndex);
      }

      while (!open.empty()) {
         int nodeIndex = *open.begin();
         found.insert(nodeIndex);
         open.erase(nodeIndex);

         auto& node = m_model.nodes[nodeIndex];
         for (int childIndex : node.children) {
            open.insert(childIndex);
         }
      }

      ozz::vector<tinygltf::Skin> skins;
      for (const tinygltf::Skin& skin : m_model.skins) {
         if (!skin.joints.empty() && found.find(skin.joints[0]) != found.end()) {
            skins.push_back(skin);
         }
      }

      return skins;
   }

   [[nodiscard]] const tinygltf::Node* GltfImporter::FindNodeByName(const std::string& name) const {
      for (const tinygltf::Node& node : m_model.nodes) {
         if (node.name == name) {
            return &node;
         }
      }

      return nullptr;
   }
}