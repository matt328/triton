#include "DrawContextFactory.hpp"

namespace tr {

DrawContextFactory::DrawContextFactory(std::shared_ptr<BufferSystem> newBufferSystem)
    : bufferSystem{std::move(newBufferSystem)} {
}

auto DrawContextFactory::createDrawContext(DrawContextConfig& config) -> Handle<DrawContext> {
  const auto handle = drawHandleGenerator.requestHandle();

  drawContextMap.emplace(handle, std::make_unique<DrawContext>(config, bufferSystem));

  return handle;
}

auto DrawContextFactory::getDrawContext(const Handle<DrawContext>& handle)
    -> std::unique_ptr<DrawContext>& {
  return drawContextMap.at(handle);
}

}
