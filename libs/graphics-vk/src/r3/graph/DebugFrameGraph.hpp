#pragma once

#include "gfx/IFrameGraph.hpp"
#include "r3/render-pass/IRenderPass.hpp"
#include "task/Frame.hpp"

namespace tr {

class Swapchain;

class DebugFrameGraph : public IFrameGraph {
public:
  explicit DebugFrameGraph(std::shared_ptr<CommandBufferManager> newCommandBufferManager,
                           std::shared_ptr<Swapchain> newSwapchain);
  ~DebugFrameGraph() override;

  DebugFrameGraph(const DebugFrameGraph&) = default;
  DebugFrameGraph(DebugFrameGraph&&) = delete;
  auto operator=(const DebugFrameGraph&) -> DebugFrameGraph& = default;
  auto operator=(DebugFrameGraph&&) -> DebugFrameGraph& = delete;

  auto addPass(std::unique_ptr<IRenderPass>&& pass) -> void override;

  [[nodiscard]] auto getPass(PassId id) -> std::unique_ptr<IRenderPass>& override;

  auto bake() -> void override;

  auto execute(const Frame* frame) -> FrameGraphResult override;

private:
  std::shared_ptr<CommandBufferManager> commandBufferManager;
  std::shared_ptr<Swapchain> swapchain;

  std::unordered_map<PassId, std::unique_ptr<IRenderPass>> passes;
};

}
