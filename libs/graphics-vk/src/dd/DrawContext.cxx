#include "DrawContext.hpp"

namespace tr {

DrawContext::DrawContext(const DrawContextCreateInfo& createInfo)
    : geometryBufferHandle(createInfo.geometryBufferHandle),
      objectDataBufferHandle(createInfo.objectDataBufferHandle),
      materialBufferHandle(createInfo.materialBufferHandle) {
}

auto DrawContext::registerRenderable([[maybe_unused]] const RenderableData& renderableData)
    -> RenderableResources {
  return {};
}

auto DrawContext::writeObjectData([[maybe_unused]] const Renderable& renderable) -> void {
}

}
