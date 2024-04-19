#pragma once

#include "ObjectData.hpp"

namespace tr::gfx {
   struct RenderData {
    public:
      RenderData() = default;
      ~RenderData() = default;

      RenderData(const RenderData&) = default;
      RenderData& operator=(const RenderData&) = default;

      RenderData(RenderData&& other) = default;
      RenderData& operator=(RenderData&&) = default;

      CameraData cameraData;
      std::vector<ObjectData> objectData;
      std::vector<MeshHandle> meshHandles;
   };
}