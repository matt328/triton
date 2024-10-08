#pragma once

#include "cm/ObjectData.hpp"

namespace tr::cm::gpu {

   struct MeshData {
      MeshHandle handle;
      size_t objectDataId;
   };

   struct RenderData {
      PushConstants pushConstants{};
      CameraData cameraData{};
      std::vector<ObjectData> objectData{};
      std::vector<MeshData> staticMeshData{};
      std::vector<MeshData> terrainMeshData{};
      std::vector<MeshData> skinnedMeshData{};
      std::vector<AnimationData> animationData{};
   };

}
