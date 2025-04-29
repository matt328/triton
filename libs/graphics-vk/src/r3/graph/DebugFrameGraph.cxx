#include "DebugFrameGraph.hpp"

namespace tr {

auto DebugFrameGraph::addPass(std::unique_ptr<GraphicsPass>&& pass, PassGraphInfo passInfo)
    -> void {
  graphicsPasses.emplace_back(std::move(pass));
}

auto DebugFrameGraph::addPass(Handle<ComputePass> passHandle, PassInfo passInfo) -> void {
}

auto DebugFrameGraph::bake() -> void {
}

auto DebugFrameGraph::execute(const Frame* frame) -> void {
  Log.trace("DebugFrameGraph::execute");

  // TODO(matt): Figure out what to do with command buffers.
  /*
    Think about the relationship of command buffers to render passes.

  */

  for (const auto& pass : graphicsPasses) {
    pass->execute(frame);
  }
}

}
