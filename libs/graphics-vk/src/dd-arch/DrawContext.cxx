#include "DrawContext.hpp"

namespace tr {

DispatchContext::DispatchContext(const DrawContextCreateInfo& createInfo)
    : id(createInfo.id),
      geometryBufferHandle(createInfo.geometryBufferHandle),
      materialBufferHandle(createInfo.materialBufferHandle) {
}

auto DispatchContext::registerRenderable([[maybe_unused]] const RenderableData& renderableData)
    -> RenderableResources {
  return {};
}

auto DispatchContext::writeObjectData([[maybe_unused]] const Renderable& renderable) -> void {
}

auto DispatchContext::record(vk::raii::CommandBuffer& cmdBuffer) -> void {
}

}
