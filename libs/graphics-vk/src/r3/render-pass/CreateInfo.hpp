#pragma once

#include "r3/ComponentIds.hpp"
#include "r3/graph/ResourceAliases.hpp"

namespace tr {

enum class RenderPassType : uint8_t {
  Forward = 0,
  Composition,
  Culling,
  Count
};

struct ForwardPassCreateInfo {
  ImageAlias colorImage;
  ImageAlias depthImage;
};

struct CullingPassCreateInfo {
  BufferAlias objectDataBuffer;
  BufferAlias geometryRegionBuffer;
  BufferAlias indirectCommandBuffer;
  BufferAlias indirectCommandCountBuffer;
};

struct CompositionPassCreateInfo {
  ImageAlias colorImage;
  ImageAlias swapchainImage;
};

using PassInfo =
    std::variant<ForwardPassCreateInfo, CullingPassCreateInfo, CompositionPassCreateInfo>;

struct RenderPassCreateInfo {
  PassId passId;
  PassInfo passInfo;
};

}
