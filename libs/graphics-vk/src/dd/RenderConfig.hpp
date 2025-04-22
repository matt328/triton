#pragma once

#include "api/gfx/Topology.hpp"
#include "as/VertexList.hpp"

namespace tr {

enum class ShadingMode : uint8_t {
  PBR = 0,
  Wireframe
};

struct Scissor {
  float x = 0.f;
  float y = 0.f;
  uint32_t width = 0;
  uint32_t height = 0;

  auto operator==(const Scissor& other) const -> bool {
    return x == other.x && y == other.y && width == other.width && height == other.height;
  }
};

struct Viewport {
  float x = 0.f;
  float y = 0.f;
  uint32_t width{};
  uint32_t height{};
  float minDepth = 0.f;
  float maxDepth = 1.f;

  auto operator==(const Viewport& other) const -> bool {
    return x == other.x && y == other.y && width == other.width && height == other.height &&
           minDepth == other.minDepth && maxDepth == other.maxDepth;
  }
};

enum class ObjectDataType : uint8_t {
  Base = 0,
  BaseMaterial,
  BaseMaterialAnimated
};

struct RenderConfig {
  VertexFormat vertexFormat;
  Topology topology;
  ShadingMode shadingMode;
  Viewport viewport;
  Scissor scissor;
  ObjectDataType objectDataType;

  auto operator==(const RenderConfig& other) const -> bool {
    return vertexFormat == other.vertexFormat && topology == other.topology &&
           shadingMode == other.shadingMode && viewport == other.viewport &&
           scissor == other.scissor && objectDataType == other.objectDataType;
  }
};

}
