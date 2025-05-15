#pragma once

#include "bk/Handle.hpp"
#include "bk/HandleGenerator.hpp"
#include "PushConstantBuilders.hpp"

namespace tr {

class DrawContext;
class DispatchContext;
class BufferSystem;
struct DrawContextConfig;
struct DispatchContextConfig;

class ContextFactory {
public:
  explicit ContextFactory(std::shared_ptr<BufferSystem> newBufferSystem);
  ~ContextFactory() = default;

  ContextFactory(const ContextFactory&) = delete;
  ContextFactory(ContextFactory&&) = delete;
  auto operator=(const ContextFactory&) -> ContextFactory& = delete;
  auto operator=(ContextFactory&&) -> ContextFactory& = delete;

  auto createDrawContext(std::string id, const DrawContextConfig& config) -> Handle<DrawContext>;
  auto getDrawContextHandle(const std::string& id) -> Handle<DrawContext>;
  auto getDrawContext(const Handle<DrawContext>& handle) -> std::unique_ptr<DrawContext>&;

  auto createDispatchContext(std::string id,
                             const DispatchContextConfig& config,
                             DispatchPushConstantsBuilder builder) -> Handle<DispatchContext>;
  auto getDispatchContextHandle(const std::string& id) -> Handle<DispatchContext>;
  auto getDispatchContext(const Handle<DispatchContext>& handle)
      -> std::unique_ptr<DispatchContext>&;

private:
  std::shared_ptr<BufferSystem> bufferSystem;

  HandleGenerator<DrawContext> drawHandleGenerator;
  std::unordered_map<Handle<DrawContext>, std::unique_ptr<DrawContext>> drawContextMap;
  std::unordered_map<std::string, Handle<DrawContext>> drawContextIdMap;

  HandleGenerator<DispatchContext> dispatchHandleGenerator;
  std::unordered_map<Handle<DispatchContext>, std::unique_ptr<DispatchContext>> dispatchContextMap;
  std::unordered_map<std::string, Handle<DispatchContext>> dispatchContextIdMap;
};

}
