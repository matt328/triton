#pragma once

#include "bk/Handle.hpp"
#include "bk/HandleGenerator.hpp"
#include "r3/ComponentIds.hpp"
#include "r3/draw-context/DispatchCreateInfos.hpp"

namespace tr {

class BufferSystem;
class IDispatchContext;
class IDrawContext;
class IShaderBindingFactory;
class IGuiCallbackRegistrar;

class ContextFactory {
public:
  explicit ContextFactory(std::shared_ptr<BufferSystem> newBufferSystem,
                          std::shared_ptr<IShaderBindingFactory> newShaderBindingFactory,
                          std::shared_ptr<IGuiCallbackRegistrar> newGuiCallbackRegistrar);
  ~ContextFactory() = default;

  ContextFactory(const ContextFactory&) = delete;
  ContextFactory(ContextFactory&&) = delete;
  auto operator=(const ContextFactory&) -> ContextFactory& = delete;
  auto operator=(ContextFactory&&) -> ContextFactory& = delete;

  auto createDispatchContext(ContextId id, DispatchCreateInfo createInfo)
      -> Handle<IDispatchContext>;
  auto getDispatchContextHandle(ContextId id) -> Handle<IDispatchContext>;

  auto getDispatchContext(const Handle<IDispatchContext>& handle)
      -> std::unique_ptr<IDispatchContext>&;

private:
  std::shared_ptr<BufferSystem> bufferSystem;
  std::shared_ptr<IShaderBindingFactory> shaderBindingFactory;
  std::shared_ptr<IGuiCallbackRegistrar> guiCallbackRegistrar;

  HandleGenerator<IDispatchContext> dispatchHandleGenerator;
  std::unordered_map<Handle<IDispatchContext>, std::unique_ptr<IDispatchContext>>
      dispatchContextMap;
  std::unordered_map<ContextId, Handle<IDispatchContext>> dispatchContextIdMap;
};

}
