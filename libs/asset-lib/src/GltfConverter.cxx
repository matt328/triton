#include "GltfConverter.hpp"

#include "GltfHelpers.hpp"

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
// #define TINYGLTF_NOEXCEPTION // optional. disable exception handling.
#include "tiny_gltf.h"

namespace tr::as::gltf {
   Converter::~Converter() noexcept {
   }

   auto Converter::convert(const std::filesystem::path& gltf, const std::filesystem::path& skeleton)
       -> Model {

      tinygltf::Model model;
      tinygltf::TinyGLTF loader;
      std::string err;
      std::string warn;

      bool ret = loader.LoadASCIIFromFile(&model, &err, &warn, gltf.string());
      if (!warn.empty()) {
         throw std::runtime_error(warn);
      }

      if (!err.empty()) {
         throw std::runtime_error(err);
      }

      if (!ret) {
         Log.error("Failed to parse glTF file");
         throw std::runtime_error("Failed to parse glTF file");
      }

      auto tritonModel = Model{};

      // Load Geometry and Textures
      const auto& scene = model.scenes[model.defaultScene];
      for (const auto& nodeIndex : scene.nodes) {
         Helpers::parseNode(model, model.nodes[nodeIndex], tritonModel);
      }

      // Load Inverse Bind Matrices
      if (model.skins.empty()) {
         throw std::runtime_error("Model file contains no skins.");
      }
      const auto& skin = model.skins[0];
      auto accessorIndex = static_cast<size_t>(skin.inverseBindMatrices);

      if (accessorIndex < 0 || accessorIndex >= model.accessors.size()) {
         throw std::runtime_error("Invalid accessor index for inverse bind matrices");
      }

      const auto& accessor = model.accessors[accessorIndex];
      const auto& bufferView = model.bufferViews[accessor.bufferView];
      const auto& buffer = model.buffers[bufferView.buffer];

      if (accessor.type != TINYGLTF_TYPE_MAT4 || accessor.count == 0) {
         throw std::runtime_error("Invalid inverse bind matrices accessor");
      }

      const auto data = reinterpret_cast<const float*>(buffer.data.data() + bufferView.byteOffset +
                                                       accessor.byteOffset);

      tritonModel.inverseBindPoses.reserve(accessor.count);

      for (size_t i = 0; i < accessor.count; ++i) {
         glm::mat4 mat = glm::make_mat4(data + i * 16);
         tritonModel.inverseBindPoses.push_back(mat);
      }

      // Calculate JointMap
      {
         ozz::io::File file(skeleton.string().c_str(), "rb");

         if (!file.opened()) {
            Log.error("Failed to open skeleton file {0}", skeleton.string());
         }

         ozz::io::IArchive archive(&file);
         if (!archive.TestTag<ozz::animation::Skeleton>()) {
            Log.error("Failed to load skeleton instance from file {0}", skeleton.string());
         }

         auto ozzSkeleton = ozz::animation::Skeleton{};
         archive >> ozzSkeleton;

         Log.trace("Skeleton has {0} joints", ozzSkeleton.num_joints());
         Log.trace("gltf Skin has {0} joints", skin.joints.size());

         int position = 0;
         for (const auto& jointIndex : skin.joints) {
            const auto& jointNode = model.nodes[jointIndex];

            const auto& jointNodeName = jointNode.name;

            int sortedIndex = -1;

            auto it = std::find(ozzSkeleton.joint_names().begin(),
                                ozzSkeleton.joint_names().end(),
                                jointNodeName);

            if (it != ozzSkeleton.joint_names().end()) {
               sortedIndex = static_cast<int>(std::distance(ozzSkeleton.joint_names().begin(), it));
            }

            tritonModel.jointRemaps.insert({position, sortedIndex});

            ++position;
         }
      }

      Log.trace("Joint Remaps Size: {0}", tritonModel.jointRemaps.size());
      for (const auto& [position, sortedIndex] : tritonModel.jointRemaps) {
         Log.trace("Joint Remap: {0}, {1}", position, sortedIndex);
      }

      Log.trace("inverseBindPoses.size(): {0}", tritonModel.inverseBindPoses.size());

      return tritonModel;
   }
}
