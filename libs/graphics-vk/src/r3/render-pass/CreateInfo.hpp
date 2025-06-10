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
  ResourceAlias colorImage;
  ResourceAlias depthImage;
};

struct CullingPassCreateInfo {
  ResourceAlias objectDataBuffer;
  ResourceAlias geometryRegionBuffer;
  ResourceAlias indirectCommandBuffer;
  ResourceAlias indirectCommandCountBuffer;
};

struct CompositionPassCreateInfo {
  ResourceAlias colorImage;
  ResourceAlias swapchainImage;
};

using PassInfo =
    std::variant<ForwardPassCreateInfo, CullingPassCreateInfo, CompositionPassCreateInfo>;

struct RenderPassCreateInfo {
  PassId passId;
  PassInfo passInfo;
};

}
