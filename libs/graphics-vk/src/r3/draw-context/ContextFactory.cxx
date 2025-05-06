#include "ContextFactory.hpp"
#include "r3/draw-context/DispatchContext.hpp"
#include "r3/draw-context/DrawContext.hpp"

namespace tr {

ContextFactory::ContextFactory(std::shared_ptr<BufferSystem> newBufferSystem)
    : bufferSystem{std::move(newBufferSystem)} {
}

auto ContextFactory::createDrawContext(std::string id, const DrawContextConfig& config)
    -> Handle<DrawContext> {
  const auto handle = drawHandleGenerator.requestHandle();

  drawContextMap.emplace(handle, std::make_unique<DrawContext>(config, bufferSystem));

  drawContextIdMap.emplace(id, handle);

  return handle;
}

auto ContextFactory::getDrawContextHandle(const std::string& id) -> Handle<DrawContext> {
  assert(drawContextIdMap.contains(id));
  return drawContextIdMap.at(id);
}

auto ContextFactory::getDrawContext(const Handle<DrawContext>& handle)
    -> std::unique_ptr<DrawContext>& {
  assert(drawContextMap.contains(handle));
  return drawContextMap.at(handle);
}

auto ContextFactory::createDispatchContext(std::string id, const DispatchContextConfig& config)
    -> Handle<DispatchContext> {
  const auto handle = dispatchHandleGenerator.requestHandle();

  dispatchContextMap.emplace(handle, std::make_unique<DispatchContext>(config, bufferSystem));

  dispatchContextIdMap.emplace(id, handle);

  return handle;
}

auto ContextFactory::getDispatchContextHandle(const std::string& id) -> Handle<DispatchContext> {
  assert(dispatchContextIdMap.contains(id));
  return dispatchContextIdMap.at(id);
}

auto ContextFactory::getDispatchContext(const Handle<DispatchContext>& handle)
    -> std::unique_ptr<DispatchContext>& {
  assert(dispatchContextMap.contains(handle));
  return dispatchContextMap.at(handle);
}

}
