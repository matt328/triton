#pragma once

#include "gfx/IFrameGraph.hpp"
#include "r3/render-pass/GraphicsPass.hpp"
#include "task/Frame.hpp"

namespace tr {

class DebugFrameGraph : public IFrameGraph {
public:
  explicit DebugFrameGraph(std::shared_ptr<CommandBufferManager> newCommandBufferManager);
  ~DebugFrameGraph() override = default;

  DebugFrameGraph(const DebugFrameGraph&) = default;
  DebugFrameGraph(DebugFrameGraph&&) = delete;
  auto operator=(const DebugFrameGraph&) -> DebugFrameGraph& = default;
  auto operator=(DebugFrameGraph&&) -> DebugFrameGraph& = delete;

  auto addPass(std::unique_ptr<GraphicsPass>&& pass, PassGraphInfo passInfo) -> void override;
  auto addPass(Handle<ComputePass> passHandle, PassInfo passInfo) -> void override;

  auto bake() -> void override;

  auto execute(const Frame* frame) -> FrameGraphResult override;

private:
  std::shared_ptr<CommandBufferManager> commandBufferManager;

  Handle<ComputePass> computePass{};
  std::vector<std::unique_ptr<GraphicsPass>> graphicsPasses;
};

}
