#include "as/gltf/GltfSkinningDataExtractor.hpp"

#include "as/Model.hpp"

namespace as {

GltfSkinningDataExtractor::~GltfSkinningDataExtractor() noexcept {
}

void GltfSkinningDataExtractor::execute(const tinygltf::Model& model,
                                        const ozz::animation::Skeleton& skeleton,
                                        Model& tritonModel) {
  if (model.skins.empty()) {
    throw std::runtime_error("Model file contains no skins.");
  }
  const auto& skin = model.skins[0];
  const auto accessorIndex = static_cast<size_t>(skin.inverseBindMatrices);

  if (accessorIndex >= model.accessors.size()) {
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
    Log.trace("Skeleton has {0} joints", skeleton.num_joints());
    Log.trace("gltf Skin has {0} joints", skin.joints.size());

    int position = 0;
    for (const auto& jointIndex : skin.joints) {
      const auto& jointNode = model.nodes[jointIndex];

      const auto& jointNodeName = jointNode.name;

      int sortedIndex = -1;

      if (const auto it = std::find(skeleton.joint_names().begin(),
                                    skeleton.joint_names().end(),
                                    jointNodeName);
          it != skeleton.joint_names().end()) {
        sortedIndex = static_cast<int>(std::distance(skeleton.joint_names().begin(), it));
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
}

}
