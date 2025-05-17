#pragma once

#include "bk/Handle.hpp"
#include "bk/HandleGenerator.hpp"
#include "r3/draw-context/DispatchCreateInfos.hpp"

namespace tr {

class BufferSystem;
class IDispatchContext;
class IDrawContext;

class ContextFactory {
public:
  explicit ContextFactory(std::shared_ptr<BufferSystem> newBufferSystem);
  ~ContextFactory() = default;

  ContextFactory(const ContextFactory&) = delete;
  ContextFactory(ContextFactory&&) = delete;
  auto operator=(const ContextFactory&) -> ContextFactory& = delete;
  auto operator=(ContextFactory&&) -> ContextFactory& = delete;

  auto createDispatchContext(std::string id, DispatchCreateInfo createInfo)
      -> Handle<IDispatchContext>;
  auto getDispatchContextHandle(const std::string& id) -> Handle<IDispatchContext>;
  auto getDispatchContext(const Handle<IDispatchContext>& handle)
      -> std::unique_ptr<IDispatchContext>&;

private:
  std::shared_ptr<BufferSystem> bufferSystem;

  HandleGenerator<IDispatchContext> dispatchHandleGenerator;
  std::unordered_map<Handle<IDispatchContext>, std::unique_ptr<IDispatchContext>>
      dispatchContextMap;
  std::unordered_map<std::string, Handle<IDispatchContext>> dispatchContextIdMap;
};

}
