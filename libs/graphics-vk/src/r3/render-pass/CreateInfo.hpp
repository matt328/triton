#pragma once

#include "bk/Handle.hpp"
#include "r3/ComponentIds.hpp"
#include "r3/graph/ImageAlias.hpp"
#include "r3/graph/ResourceAliases.hpp"

namespace tr {

class IShaderBinding;
class DSLayout;

enum class RenderPassType : uint8_t {
  Forward = 0,
  Composition,
  Culling,
  ImGui,
  Count
};

struct ImGuiPassCreateInfo {
  ImageAlias colorImage;
};

struct ForwardPassCreateInfo {
  ImageAlias colorImage;
  ImageAlias depthImage;
  std::vector<Handle<DSLayout>> dsLayoutHandles;
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
  LogicalHandle<IShaderBinding> defaultShaderBinding;
  Handle<DSLayout> defaultDSLayout;
};

using PassInfo = std::variant<ForwardPassCreateInfo,
                              CullingPassCreateInfo,
                              CompositionPassCreateInfo,
                              ImGuiPassCreateInfo>;

struct RenderPassCreateInfo {
  PassId passId;
  PassInfo passInfo;
};

}
