#pragma once

namespace al::gltf {
   class GltfImporter : public ozz::animation::offline::OzzImporter {

    public:
      GltfImporter();

    private:
      tinygltf::TinyGLTF m_loader;
      tinygltf::Model m_model;

      bool Load(const char* _filename) override;

      void FindSkinRootJointIndices(const ozz::vector<tinygltf::Skin>& skins,
                                    ozz::vector<int>& roots);

      bool Import(ozz::animation::offline::RawSkeleton* _skeleton, const NodeType& _types) override;

      bool Import(const char* _animation_name,
                  const ozz::animation::Skeleton& skeleton,
                  float _sampling_rate,
                  ozz::animation::offline::RawAnimation* _animation) override;

      bool ImportNode(const tinygltf::Node& _node,
                      ozz::animation::offline::RawSkeleton::Joint* _joint);

      AnimationNames GetAnimationNames() override;

      bool SampleAnimationChannel(const tinygltf::Model& _model,
                                  const tinygltf::AnimationSampler& _sampler,
                                  const std::string& _target_path,
                                  float _sampling_rate,
                                  float* _duration,
                                  ozz::animation::offline::RawAnimation::JointTrack* _track);

      [[nodiscard]] ozz::vector<tinygltf::Skin> GetSkinsForScene(
          const tinygltf::Scene& _scene) const;

      [[nodiscard]] const tinygltf::Node* FindNodeByName(const std::string& _name) const;

      // no support for user-defined tracks
      NodeProperties GetNodeProperties(const char*) override {
         return {};
      }
      bool Import(const char*,
                  const char*,
                  const char*,
                  NodeProperty::Type,
                  float,
                  ozz::animation::offline::RawFloatTrack*) override {
         return false;
      }
      bool Import(const char*,
                  const char*,
                  const char*,
                  NodeProperty::Type,
                  float,
                  ozz::animation::offline::RawFloat2Track*) override {
         return false;
      }
      bool Import(const char*,
                  const char*,
                  const char*,
                  NodeProperty::Type,
                  float,
                  ozz::animation::offline::RawFloat3Track*) override {
         return false;
      }
      bool Import(const char*,
                  const char*,
                  const char*,
                  NodeProperty::Type,
                  float,
                  ozz::animation::offline::RawFloat4Track*) override {
         return false;
      }
   };
}