#include "ContextFactory.hpp"
#include "r3/ComponentIds.hpp"
#include "r3/draw-context/ForwardDrawContext.hpp"
#include "r3/draw-context/IDispatchContext.hpp"
#include "r3/draw-context/CullingDispatchContext.hpp"

namespace tr {

ContextFactory::ContextFactory(std::shared_ptr<BufferSystem> newBufferSystem)
    : bufferSystem{std::move(newBufferSystem)} {
}

auto ContextFactory::createDispatchContext(ContextId id, DispatchCreateInfo createInfo)
    -> Handle<IDispatchContext> {
  const auto handle = dispatchHandleGenerator.requestHandle();

  std::visit(
      [&](auto&& ci) {
        using T = std::decay_t<decltype(ci)>;
        if constexpr (std::is_same_v<T, CullingDispatchContextCreateInfo>) {
          dispatchContextMap.emplace(
              handle,
              std::make_unique<CullingDispatchContext>(id,
                                                       bufferSystem,
                                                       std::forward<decltype(ci)>(ci)));
        }
        if constexpr (std::is_same_v<T, ForwardDrawContextCreateInfo>) {
          dispatchContextMap.emplace(
              handle,
              std::make_unique<ForwardDrawContext>(id,
                                                   bufferSystem,
                                                   std::forward<decltype(ci)>(ci)));
        }
      },
      std::move(createInfo));
  dispatchContextIdMap.emplace(id, handle);
  return handle;
}

auto ContextFactory::getDispatchContextHandle(ContextId id) -> Handle<IDispatchContext> {
  assert(dispatchContextIdMap.contains(id));
  return dispatchContextIdMap.at(id);
}

auto ContextFactory::getDispatchContext(const Handle<IDispatchContext>& handle)
    -> std::unique_ptr<IDispatchContext>& {
  assert(dispatchContextMap.contains(handle));
  return dispatchContextMap.at(handle);
}

}
