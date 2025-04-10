#include "DrawContext.hpp"

namespace tr {

DrawContext::DrawContext(const DrawContextCreateInfo& createInfo)
    : id(createInfo.id),
      geometryBufferHandle(createInfo.geometryBufferHandle),
      materialBufferHandle(createInfo.materialBufferHandle) {
}

auto DrawContext::registerRenderable([[maybe_unused]] const RenderableData& renderableData)
    -> RenderableResources {
  return {};
}

auto DrawContext::writeObjectData([[maybe_unused]] const Renderable& renderable) -> void {
}

}
