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

   using ModelHandle = std::unordered_map<MeshHandle, TextureHandle>;

   // TODO: Find a new home for this struct
   struct LoadedSkinnedModelData {
      MeshHandle meshHandle;
      TextureHandle textureHandle;
      SkeletonHandle skeletonHandle;
      AnimationHandle animationHandle;
      std::unordered_map<int, int> jointMap;
      std::vector<glm::mat4> inverseBindMatrices;
   };

   using MeshHandles = std::unordered_map<cm::MeshHandle, cm::TextureHandle>;
}
