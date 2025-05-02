#include "DebugFrameGraph.hpp"
#include "r3/render-pass/ComputePass.hpp"

namespace tr {

DebugFrameGraph::DebugFrameGraph(std::shared_ptr<CommandBufferManager> newCommandBufferManager)
    : commandBufferManager{std::move(newCommandBufferManager)} {
}

DebugFrameGraph::~DebugFrameGraph() {
  graphicsPasses.clear();
}

auto DebugFrameGraph::addPass(std::unique_ptr<GraphicsPass>&& pass, PassGraphInfo passInfo)
    -> void {
  graphicsPasses.emplace_back(std::move(pass));
}

auto DebugFrameGraph::addPass(std::unique_ptr<ComputePass>&& pass, PassGraphInfo passInfo) -> void {
  computePasses.emplace_back(std::move(pass));
}

auto DebugFrameGraph::bake() -> void {
}

/*
  TODO(matt): Figure out where the passBindFunction comes from.
  Renderer knows about the shape of the push constants, that can be passed into the config.
  But what knows about the buffer addresses that are mapped into the PushConstants structs
  I think this is the meat of a Draw/DispatchContext.

  I think we can use pushConstants2 with vk::PushConstantsInfo struct.
  This avoids having a specific struct for PushConstants, and just treating them as data as a void*

  Should be able to declaratively define what all logical buffers' addresses are used by a
  drawcontext and then iterate that list and just accumulate the address data in
  std::vector<uint8_t> blob to be set in vk::PushConstantsInfo

*/

auto DebugFrameGraph::execute(const Frame* frame) -> FrameGraphResult {
  auto frameGraphResult = FrameGraphResult{};

  for (const auto& pass : computePasses) {
    const auto request = CommandBufferRequest{.threadId = std::this_thread::get_id(),
                                              .frameId = frame->getIndex(),
                                              .passId = pass->getId(),
                                              .queueType = QueueType::Compute};
    auto& commandBuffer = commandBufferManager->requestCommandBuffer(request);
    pass->dispatch(frame, commandBuffer, passBindFunction);
    frameGraphResult.commandBuffers.push_back(&(*commandBuffer));
  }

  for (const auto& pass : graphicsPasses) {
    const auto request = CommandBufferRequest{.threadId = std::this_thread::get_id(),
                                              .frameId = frame->getIndex(),
                                              .passId = pass->getId(),
                                              .queueType = QueueType::Graphics};
    auto& commandBuffer = commandBufferManager->requestCommandBuffer(request);
    pass->execute(frame, commandBuffer, passBindFunction);
    frameGraphResult.commandBuffers.push_back(&(*commandBuffer));
  }
  return frameGraphResult;
}

}
