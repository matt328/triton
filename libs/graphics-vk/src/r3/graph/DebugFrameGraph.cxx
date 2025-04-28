#include "DebugFrameGraph.hpp"

namespace tr {

auto DebugFrameGraph::addPass(const std::unique_ptr<GraphicsPass>&& pass, PassGraphInfo passInfo)
    -> void {
}

auto DebugFrameGraph::addPass(Handle<ComputePass> passHandle, PassInfo passInfo) -> void {
}

auto DebugFrameGraph::bake() -> void {
}

auto DebugFrameGraph::execute(const Frame* frame) -> void {
  Log.trace("DebugFrameGraph::execute");
}

}
