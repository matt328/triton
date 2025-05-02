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
