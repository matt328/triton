#pragma once

#include "ObjectData.hpp"

namespace tr::gfx {

   struct MeshData {
      MeshHandle handle;
      size_t objectDataId;
   };

   struct RenderData {
      PushConstants pushConstants;
      CameraData cameraData;
      std::vector<ObjectData> objectData;
      std::vector<MeshData> staticMeshData;
      std::vector<MeshData> terrainMeshData;
   };

}
