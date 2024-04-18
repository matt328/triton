#pragma once

#include "ObjectData.hpp"

namespace tr::gfx {
   struct RenderData {
    public:
      RenderData() = default;
      ~RenderData() = default;

      RenderData(const RenderData&) = delete;
      RenderData& operator=(const RenderData&) = delete;

      RenderData(RenderData&& other) noexcept
          : cameraData{std::move(other.cameraData)},
            objectData{std::move(other.objectData)},
            meshHandles{std::move(other.meshHandles)} {
      }
      RenderData& operator=(RenderData&&) = default;

      CameraData cameraData;
      std::vector<ObjectData> objectData;
      std::vector<MeshHandle> meshHandles;
   };
}