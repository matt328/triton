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

class DispatchContext {
public:
  explicit DispatchContext(const DrawContextCreateInfo& createInfo);
  ~DispatchContext() = default;

  DispatchContext(const DispatchContext&) = delete;
  DispatchContext(DispatchContext&&) = delete;
  auto operator=(const DispatchContext&) -> DispatchContext& = delete;
  auto operator=(DispatchContext&&) -> DispatchContext& = delete;

  auto registerRenderable(const RenderableData& renderableData) -> RenderableResources;
  auto writeObjectData(const Renderable& renderable) -> void;
  auto record(vk::raii::CommandBuffer& cmdBuffer) -> void;

private:
  [[maybe_unused]] size_t id;
  [[maybe_unused]] BufferHandle geometryBufferHandle;
  [[maybe_unused]] BufferHandle materialBufferHandle;
  [[maybe_unused]] BufferHandle geometryRegionBufferHandle;

  [[maybe_unused]] LogicalBufferHandle objectDataBufferHandle;
  [[maybe_unused]] LogicalBufferHandle objectDataIndexBufferHandle;
  [[maybe_unused]] LogicalBufferHandle objectCountBufferHandle;
  [[maybe_unused]] LogicalBufferHandle indirectDrawCommandBufferHandle;
};

}
