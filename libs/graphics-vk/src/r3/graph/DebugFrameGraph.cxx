#include "DebugFrameGraph.hpp"
#include "r3/render-pass/ComputePass.hpp"
#include "vk/core/Swapchain.hpp"

namespace tr {

DebugFrameGraph::DebugFrameGraph(std::shared_ptr<CommandBufferManager> newCommandBufferManager,
                                 std::shared_ptr<Swapchain> newSwapchain)
    : commandBufferManager{std::move(newCommandBufferManager)}, swapchain{std::move(newSwapchain)} {
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
  ZoneScopedN("DebugFrameGraph::execute");
  auto frameGraphResult = FrameGraphResult{};

  for (const auto& pass : computePasses | std::views::values) {
    const auto request = CommandBufferRequest{.threadId = std::this_thread::get_id(),
                                              .frameId = frame->getIndex(),
                                              .passId = pass->getId(),
                                              .queueType = QueueType::Graphics};
    auto& commandBuffer = commandBufferManager->requestCommandBuffer(request);
    pass->dispatch(frame, commandBuffer);
    frameGraphResult.commandBuffers.push_back(commandBuffer);
  }

  for (const auto& pass : graphicsPasses | std::views::values) {
    const auto request = CommandBufferRequest{.threadId = std::this_thread::get_id(),
                                              .frameId = frame->getIndex(),
                                              .passId = pass->getId(),
                                              .queueType = QueueType::Graphics};
    auto& commandBuffer = commandBufferManager->requestCommandBuffer(request);
    commandBuffer.begin(
        vk::CommandBufferBeginInfo{.flags = vk::CommandBufferUsageFlagBits::eSimultaneousUse});
    pass->execute(frame, commandBuffer);

    auto swapchainImage = swapchain->getSwapchainImage(frame->getSwapchainImageIndex());

    vk::ImageSubresourceRange fullRange{
        .aspectMask = vk::ImageAspectFlagBits::eColor,
        .baseMipLevel = 0,
        .levelCount = 1, // replace with actual mip levels
        .baseArrayLayer = 0,
        .layerCount = 1 // replace with actual array layers
    };

    vk::ImageMemoryBarrier2 presentBarrier{
        .srcStageMask = vk::PipelineStageFlagBits2::eColorAttachmentOutput,
        .srcAccessMask = vk::AccessFlagBits2::eColorAttachmentWrite,
        .dstStageMask = vk::PipelineStageFlagBits2::eBottomOfPipe,
        .dstAccessMask = {},
        .oldLayout = vk::ImageLayout::eUndefined,
        .newLayout = vk::ImageLayout::ePresentSrcKHR,
        .image = swapchainImage,
        .subresourceRange = fullRange};

    vk::DependencyInfo info{.imageMemoryBarrierCount = 1, .pImageMemoryBarriers = &presentBarrier};

    commandBuffer.pipelineBarrier2(info);
    commandBuffer.end();
    frameGraphResult.commandBuffers.push_back(commandBuffer);
  }
  return frameGraphResult;
}

}
