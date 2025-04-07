#pragma once

#include "api/gw/RenderableData.hpp"
#include "api/gw/RenderableResources.hpp"
#include "dd/Renderable.hpp"
#include "vk/ResourceManagerHandles.hpp"

namespace tr {

class DrawContext {
public:
  // Create Constructor so that handles are immutable
  DrawContext() = default;
  ~DrawContext() = default;

  DrawContext(const DrawContext&) = delete;
  DrawContext(DrawContext&&) = delete;
  auto operator=(const DrawContext&) -> DrawContext& = delete;
  auto operator=(DrawContext&&) -> DrawContext& = delete;

  auto registerRenderable(const RenderableData& renderableData) -> RenderableResources;
  auto writeObjectData(const Renderable& renderable) -> void;

private:
  std::unique_ptr<BufferHandle> vertexBufferHandle;
  std::unique_ptr<BufferHandle> indexBufferHandle;
  std::unique_ptr<BufferHandle> objectDataBufferHandle;
};

}
