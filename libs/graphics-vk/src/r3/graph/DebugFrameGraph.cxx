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
/*
  Plan for developing framegraph:
  Keep single threaded, but use a PassContext as a 'Task'.
  - PassContexts will be craeted at bake() and each create their own CommandPool
  - When asked to execute a Graphics/ComputePass, they'll allocate a command buffer from the pool,
  record it, and return it in a PassResult struct
  - The FrameGraph will iterate all the Passes, asking PassContexts to execute them, and
  accumulating their PassResults.
  - Combine the CommandBuffers from the PassResults into an array and return that to the renderer so
  it can submit them.
  CommandBuffer/Pool
  - Set of pools per thread (PassGroup)
  - Each Thread level group gets 1 pool per frame in flight.
  - might only initially have 1 command buffer per pool, but that's ok for now, there will be more
  later.
*/
auto DebugFrameGraph::execute(const Frame* frame) -> void {
  Log.trace("DebugFrameGraph::execute");

  std::vector<vk::raii::CommandBuffer*> commandBuffers{};

  for (const auto& pass : graphicsPasses) {
    auto& commandBuffer = commandBufferManager->getCommandBuffer(pass->getId());
    pass->execute(frame, commandBuffer);
    commandBuffers.push_back(&commandBuffer);
  }
  return commandBuffers;
}

}
