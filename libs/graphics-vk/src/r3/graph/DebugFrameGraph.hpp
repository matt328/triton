#pragma once

#include "gfx/IFrameGraph.hpp"
#include "r3/render-pass/GraphicsPass.hpp"
#include "task/Frame.hpp"

namespace tr {

struct PassContext {
  size_t passGroupId; // Eventually one PassGroup per thread

  auto executeRenderPass(std::vector<GraphicsPass>& passes, const Frame* frame)
      -> vk::raii::CommandBuffer& {
    auto& commandBuffer = mgr->getCommandBuffer(passGroupId, frame->getIndex());
    for (const auto& pass : passes) {
      pass.execute(frame, commandBuffer);
    }
  }
};

class DebugFrameGraph : public IFrameGraph {
public:
  DebugFrameGraph() = default;
  ~DebugFrameGraph() override = default;

  DebugFrameGraph(const DebugFrameGraph&) = default;
  DebugFrameGraph(DebugFrameGraph&&) = delete;
  auto operator=(const DebugFrameGraph&) -> DebugFrameGraph& = default;
  auto operator=(DebugFrameGraph&&) -> DebugFrameGraph& = delete;

  auto addPass(std::unique_ptr<GraphicsPass>&& pass, PassGraphInfo passInfo) -> void override;
  auto addPass(Handle<ComputePass> passHandle, PassInfo passInfo) -> void override;

  auto bake() -> void override;

  auto execute(const Frame* frame) -> void override;

private:
  Handle<ComputePass> computePass{};
  std::vector<std::unique_ptr<GraphicsPass>> graphicsPasses;
};

}
