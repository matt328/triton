#pragma once

#include "Handles.hpp"
#include "cm/ObjectData.hpp"

namespace tr::cm::gpu {

   struct MeshData {
      MeshHandle handle{};
      cm::Topology topology{cm::Topology::Triangles};
      size_t objectDataId{};

      friend void swap(MeshData& first, MeshData& second) noexcept {
         using std::swap;
         swap(first.handle, second.handle);
         swap(first.topology, second.topology);
         swap(first.objectDataId, second.objectDataId);
      }
   };

   struct alignas(16) GpuInstanceData {
      glm::mat4 modelMatrix;
      alignas(4) uint32_t visible;
   };

   struct RenderData {
      PushConstants pushConstants{};
      CameraData cameraData{};
      std::vector<ObjectData> objectData{};
      std::vector<MeshData> staticMeshData{};
      std::vector<MeshData> terrainMeshData{};
      std::vector<MeshData> skinnedMeshData{};
      std::vector<AnimationData> animationData{};
      std::unordered_map<size_t, std::vector<GpuInstanceData>> instanceData{};
   };

}
