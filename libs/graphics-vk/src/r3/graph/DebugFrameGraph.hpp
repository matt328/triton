#pragma once

#include "gfx/IFrameGraph.hpp"
#include "r3/render-pass/GraphicsPass.hpp"
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

  auto addPass(std::unique_ptr<GraphicsPass>&& pass, PassGraphInfo passInfo) -> void override;
  auto addPass(std::unique_ptr<ComputePass>&& pass, PassGraphInfo passInfo) -> void override;

  [[nodiscard]] auto getGraphicsPass(std::string id) -> std::unique_ptr<GraphicsPass>& override;

  [[nodiscard]] auto getComputePass(std::string id) -> std::unique_ptr<ComputePass>& override;

  auto bake() -> void override;

  auto execute(const Frame* frame) -> FrameGraphResult override;

private:
  std::shared_ptr<CommandBufferManager> commandBufferManager;
  std::shared_ptr<Swapchain> swapchain;

  std::unordered_map<std::string, std::unique_ptr<ComputePass>> computePasses;
  std::unordered_map<std::string, std::unique_ptr<GraphicsPass>> graphicsPasses;
};

}
