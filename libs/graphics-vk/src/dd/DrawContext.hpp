#pragma once

#include "api/gw/RenderableData.hpp"
#include "api/gw/RenderableResources.hpp"
#include "dd/Renderable.hpp"
#include "vk/ResourceManagerHandles.hpp"

namespace tr {

struct DrawContextCreateInfo {
  BufferHandle geometryBufferHandle;
  BufferHandle objectDataBufferHandle;
  BufferHandle materialBufferHandle;
};

class DrawContext {
public:
  explicit DrawContext(const DrawContextCreateInfo& creatInfo);
  ~DrawContext() = default;

  DrawContext(const DrawContext&) = delete;
  DrawContext(DrawContext&&) = delete;
  auto operator=(const DrawContext&) -> DrawContext& = delete;
  auto operator=(DrawContext&&) -> DrawContext& = delete;

  auto registerRenderable(const RenderableData& renderableData) -> RenderableResources;
  auto writeObjectData(const Renderable& renderable) -> void;

private:
  BufferHandle geometryBufferHandle;
  BufferHandle objectDataBufferHandle;
  BufferHandle materialBufferHandle;
};

}
