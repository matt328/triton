#pragma once

#include "api/gw/RenderableData.hpp"
#include "api/gw/RenderableResources.hpp"
#include "dd/LogicalBufferHandle.hpp"
#include "dd/Renderable.hpp"
#include "vk/ResourceManagerHandles.hpp"

namespace tr {

struct DrawContextCreateInfo {
  size_t id;
  // Shared Buffers
  BufferHandle geometryBufferHandle;
  BufferHandle materialBufferHandle;
  BufferHandle geometryRegionBufferHandle;
  // Per Frame Buffers
  LogicalBufferHandle objectDataBufferHandle;
  LogicalBufferHandle objectDataIndexBufferHandle;
  LogicalBufferHandle objectCountBufferHandle;
  LogicalBufferHandle indirectDrawCommandBufferHandle;
};

class DrawContext {
public:
  explicit DrawContext(const DrawContextCreateInfo& createInfo);
  ~DrawContext() = default;

  DrawContext(const DrawContext&) = delete;
  DrawContext(DrawContext&&) = delete;
  auto operator=(const DrawContext&) -> DrawContext& = delete;
  auto operator=(DrawContext&&) -> DrawContext& = delete;

  auto registerRenderable(const RenderableData& renderableData) -> RenderableResources;
  auto writeObjectData(const Renderable& renderable) -> void;

private:
  size_t id;
  [[maybe_unused]] BufferHandle geometryBufferHandle;
  [[maybe_unused]] BufferHandle materialBufferHandle;
  [[maybe_unused]] BufferHandle geometryRegionBufferHandle;

  [[maybe_unused]] LogicalBufferHandle objectDataBufferHandle;
  [[maybe_unused]] LogicalBufferHandle objectDataIndexBufferHandle;
  [[maybe_unused]] LogicalBufferHandle objectCountBufferHandle;
  [[maybe_unused]] LogicalBufferHandle indirectDrawCommandBufferHandle;
};

}
