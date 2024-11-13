#pragma once

namespace tr::cm {
   using TextureHandle = size_t;
   using MeshHandle = size_t;
   using MaterialHandle = uint32_t;
   using ModelInfoHandle = uint32_t;
   using JointMatricesHandle = uint32_t;
   using MeshMaterialHandle = std::unordered_map<MaterialHandle, std::vector<MeshHandle>>;

   using SkeletonHandle = uint64_t;
   using AnimationHandle = uint64_t;

   struct LoadedModelData {
      MeshHandle meshHandle{};
      TextureHandle textureHandle{};
   };

   enum class Topology : uint8_t {
      Triangles = 0,
      LineList = 1
   };

   struct MeshData {
      MeshHandle meshHandle{};
      Topology topology{Topology::Triangles};
      TextureHandle textureHandle{};
   };

   struct SkinData {
      std::unordered_map<int, int> jointMap{};
      std::vector<glm::mat4> inverseBindMatrices{};
   };

   struct AnimationData {
      SkeletonHandle skeletonHandle{};
      AnimationHandle animationHandle{};
   };

   struct ModelData {
      MeshData meshData{};
      std::optional<SkinData> skinData{};
      std::optional<AnimationData> animationData{};
   };

   struct GroupHandle {
      size_t meshId;
      size_t instanceId;
   };
}
