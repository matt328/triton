#pragma once

#include "IFrameGraph.hpp"

namespace tr {

class DebugFrameGraph : public IFrameGraph {
public:
  DebugFrameGraph() = default;
  ~DebugFrameGraph() override = default;

  DebugFrameGraph(const DebugFrameGraph&) = default;
  DebugFrameGraph(DebugFrameGraph&&) = delete;
  auto operator=(const DebugFrameGraph&) -> DebugFrameGraph& = default;
  auto operator=(DebugFrameGraph&&) -> DebugFrameGraph& = delete;

  auto addPass(Handle<GraphicsPass> passHandle, PassInfo passInfo) -> void override;
  auto addPass(Handle<ComputePass> passHandle, PassInfo passInfo) -> void override;

  auto bake() -> void override;

  auto execute(const Frame* frame) -> void override;

private:
  Handle<ComputePass> computePass{};
  std::vector<Handle<GraphicsPass>> graphicsPasses;
};

}
