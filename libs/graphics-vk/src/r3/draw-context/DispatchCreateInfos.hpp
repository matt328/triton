#pragma once

#include "buffers/ManagedBuffer.hpp"

namespace tr {
struct CullingDispatchContextCreateInfo {
  LogicalHandle<ManagedBuffer> objectData;
  LogicalHandle<ManagedBuffer> geometryRegion;
  LogicalHandle<ManagedBuffer> objectPositions;
  LogicalHandle<ManagedBuffer> objectRotations;
  LogicalHandle<ManagedBuffer> objectScales;
  LogicalHandle<ManagedBuffer> indirectCommand;
  LogicalHandle<ManagedBuffer> indirectCount;
  Handle<ManagedBuffer> indexData;
  Handle<ManagedBuffer> vertexPosition;
  Handle<ManagedBuffer> vertexNormal;
  Handle<ManagedBuffer> vertexTexCoord;
  Handle<ManagedBuffer> vertexColor;
};

struct ForwardDrawContextCreateInfo {
  vk::Viewport viewport;
  vk::Rect2D scissor;
  LogicalHandle<ManagedBuffer> objectData;
  LogicalHandle<ManagedBuffer> objectPositions;
  LogicalHandle<ManagedBuffer> objectRotations;
  LogicalHandle<ManagedBuffer> objectScales;
  LogicalHandle<ManagedBuffer> geometryRegion;
  Handle<ManagedBuffer> indexData;
  Handle<ManagedBuffer> vertexPosition;
  Handle<ManagedBuffer> vertexTexCoord;
  Handle<ManagedBuffer> vertexColor;
  Handle<ManagedBuffer> vertexNormal;
  LogicalHandle<ManagedBuffer> indirectCommand;
  LogicalHandle<ManagedBuffer> indirectCount;
};

struct CompositionContextCreateInfo {};

using DispatchCreateInfo = std::variant<CullingDispatchContextCreateInfo,
                                        ForwardDrawContextCreateInfo,
                                        CompositionContextCreateInfo>;

}
