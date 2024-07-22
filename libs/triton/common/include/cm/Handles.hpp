#pragma once

namespace tr::cm {
   using TextureHandle = uint32_t;
   using MeshHandle = size_t;
   using MaterialHandle = uint32_t;
   using ModelInfoHandle = uint32_t;
   using JointMatricesHandle = uint32_t;
   using MeshMaterialHandle = std::unordered_map<MaterialHandle, std::vector<MeshHandle>>;

   using SkeletonHandle = uint64_t;
   using AnimationHandle = uint64_t;

   struct LoadedModelData {
      MeshHandle meshHandle;
      TextureHandle textureHandle;
   };

   struct SkinnedLoadedModelData : public LoadedModelData {
      SkeletonHandle skeletonHandle;
      AnimationHandle animationHandle;
      std::unordered_map<int, int> jointMap;
      std::vector<glm::mat4> inverseBindMatrices;
   };
}
