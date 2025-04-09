#pragma once

#include "api/gw/RenderableData.hpp"
#include "api/gw/RenderableResources.hpp"
#include "dd/LogicalBufferHandle.hpp"
#include "dd/Renderable.hpp"
#include "vk/ResourceManagerHandles.hpp"

namespace tr {

struct DrawContextCreateInfo {
  BufferHandle geometryBufferHandle;
  LogicalBufferHandle objectDataBufferHandle;
  BufferHandle materialBufferHandle;
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
  [[maybe_unused]] BufferHandle geometryBufferHandle;
  [[maybe_unused]] BufferHandle objectDataBufferHandle;
  [[maybe_unused]] BufferHandle materialBufferHandle;
};

}
