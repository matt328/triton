#pragma once

#include "buffers/ManagedBuffer.hpp"

namespace tr {
struct CullingDispatchContextCreateInfo {
  LogicalHandle<ManagedBuffer> objectData;
  LogicalHandle<ManagedBuffer> indirectCommand;
  LogicalHandle<ManagedBuffer> indirectCount;
  Handle<ManagedBuffer> geometryRegion;
  Handle<ManagedBuffer> indexData;
  Handle<ManagedBuffer> vertexPosition;
  Handle<ManagedBuffer> vertexNormal;
  Handle<ManagedBuffer> vertexTexCoord;
  Handle<ManagedBuffer> vertexColor;
};

struct ForwardDrawContextCreateInfo {
  vk::Viewport viewport;
  vk::Rect2D scissor;
};

using DispatchCreateInfo =
    std::variant<CullingDispatchContextCreateInfo, ForwardDrawContextCreateInfo>;

}
