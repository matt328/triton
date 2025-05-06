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
  graphicsPasses.emplace(passInfo.id, std::move(pass));
}

auto DebugFrameGraph::addPass(std::unique_ptr<ComputePass>&& pass, PassGraphInfo passInfo) -> void {
  computePasses.emplace(passInfo.id, std::move(pass));
}

[[nodiscard]] auto DebugFrameGraph::getGraphicsPass(std::string id)
    -> std::unique_ptr<GraphicsPass>& {
  assert(graphicsPasses.contains(id) && "Requested a graphics pass that doesn't exist");
  return graphicsPasses.at(id);
}

[[nodiscard]] auto DebugFrameGraph::getComputePass(std::string id)
    -> std::unique_ptr<ComputePass>& {
  assert(computePasses.contains(id) && "Requested a compute pass that doesn't exist");
  return computePasses.at(id);
}

auto DebugFrameGraph::bake() -> void {
}

auto DebugFrameGraph::execute(const Frame* frame) -> FrameGraphResult {
  auto frameGraphResult = FrameGraphResult{};

  for (const auto& pass : computePasses | std::views::values) {
    const auto request = CommandBufferRequest{.threadId = std::this_thread::get_id(),
                                              .frameId = frame->getIndex(),
                                              .passId = pass->getId(),
                                              .queueType = QueueType::Compute};
    auto& commandBuffer = commandBufferManager->requestCommandBuffer(request);
    pass->dispatch(frame, commandBuffer);
    frameGraphResult.commandBuffers.push_back(&(*commandBuffer));
  }

  for (const auto& pass : graphicsPasses | std::views::values) {
    const auto request = CommandBufferRequest{.threadId = std::this_thread::get_id(),
                                              .frameId = frame->getIndex(),
                                              .passId = pass->getId(),
                                              .queueType = QueueType::Graphics};
    auto& commandBuffer = commandBufferManager->requestCommandBuffer(request);
    pass->execute(frame, commandBuffer);
    frameGraphResult.commandBuffers.push_back(&(*commandBuffer));
  }
  return frameGraphResult;
}

}
