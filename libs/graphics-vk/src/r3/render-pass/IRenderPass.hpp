#pragma once

#include "bk/Handle.hpp"
#include "r3/ComponentIds.hpp"
#include "r3/graph/IGraphInfoProvider.hpp"

namespace tr {

class Frame;
class IDispatchContext;

class IRenderPass : public IGraphInfoProvider {
public:
  IRenderPass() = default;
  virtual ~IRenderPass() override = default;

  IRenderPass(const IRenderPass&) = default;
  IRenderPass(IRenderPass&&) = delete;
  auto operator=(const IRenderPass&) -> IRenderPass& = default;
  auto operator=(IRenderPass&&) -> IRenderPass& = delete;

  [[nodiscard]] virtual auto getId() const -> PassId = 0;
  virtual auto execute(const Frame* frame, vk::raii::CommandBuffer& cmdBuffer) -> void = 0;
  virtual auto registerDispatchContext(Handle<IDispatchContext> handle) -> void = 0;
};

}
