#include "DefaultRenderScheduler.hpp"
#include "CommandBufferManager.hpp"
#include "Maths.hpp"

#include "gfx/QueueTypes.hpp"
#include "task/Frame.hpp"
#include "task/IRenderTask.hpp"
#include "task/graph/TaskGraph.hpp"

/*
   Each frame really only needs one primary command buffer. Secondary command buffers are for
   when you render stuff that doesn't change often, so you don't have to re record them every frame,
   or for when you want to render the same stuff multiple times with different attachments, etc.
   reuse the same secondary command buffer to render the 3 components of deferred rendering's
   gbuffer
*/

namespace tr {

DefaultRenderScheduler::DefaultRenderScheduler(
    std::shared_ptr<IFrameManager> newFrameManager,
    std::shared_ptr<CommandBufferManager> newCommandBufferManager,
    std::shared_ptr<queue::Graphics> newGraphicsQueue,
    std::shared_ptr<VkResourceManager> newResourceManager,
    std::shared_ptr<Swapchain> newSwapchain,
    std::shared_ptr<RenderTaskFactory> newRenderTaskFactory,
    std::shared_ptr<TaskGraph> newTaskGraph,
    const RenderContextConfig& rendererConfig)
    : frameManager{std::move(newFrameManager)},
      commandBufferManager{std::move(newCommandBufferManager)},
      graphicsQueue{std::move(newGraphicsQueue)},
      resourceManager{std::move(newResourceManager)},
      swapchain{std::move(newSwapchain)},
      renderTaskFactory{std::move(newRenderTaskFactory)},
      taskGraph{std::move(newTaskGraph)} {

  commandBufferManager->registerType(PoolId::Main);

  const auto drawImageExtent = vk::Extent2D{
      .width = maths::scaleNumber(swapchain->getImageExtent().width, rendererConfig.renderScale),
      .height = maths::scaleNumber(swapchain->getImageExtent().height, rendererConfig.renderScale)};

  resourceManager->createDepthImageAndView(DepthImageName,
                                           drawImageExtent,
                                           swapchain->getDepthFormat());

  cubeRenderTask = renderTaskFactory->createCubeRenderTask();
  /*auto cullingTask = std::make_shared<CullingRenderTask>();*/

  // The tasks themselves will create a ResourceUsage.
  // These buffers here need to be some indirection into the current frame's buffers
  /*cullingTask->registerResource("objectBoundsBuffer", objectBoundsBuffer);*/
  /*cullingTask->registerResource("indirectCommandBuffer", indirectCommandBuffer);*/
  /*cullingTask->registerResource("drawCountBuffer", drawCountBuffer);*/
  /**/
  cubeRenderTask->registerResource(CubeRenderTask::ResourceSlot::DrawCountBuffer,
                                   "drawCountBuffer");
  cubeRenderTask->registerResource(CubeRenderTask::ResourceSlot::IndirectBuffer,
                                   "indirectCommandBuffer");
}

DefaultRenderScheduler::~DefaultRenderScheduler() {
  Log.trace("Destroying DefaultRenderScheduler");
}

auto DefaultRenderScheduler::executeStaticTasks(Frame& frame) const -> void {
  auto& commandBuffer = frame.getCommandBuffer(CmdBufferType::Main);

  const auto colorAttachmentInfo = vk::RenderingAttachmentInfo{
      .imageView = resourceManager->getImageView(frame.getDrawImageId()),
      .imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
      .loadOp = vk::AttachmentLoadOp::eClear,
      .storeOp = vk::AttachmentStoreOp::eStore,
      .clearValue = vk::ClearValue{.color = vk::ClearColorValue{std::array<float, 4>(
                                       {{0.39f, 0.58f, 0.93f, 1.f}})}},
  };

  const auto depthAttachmentInfo = vk::RenderingAttachmentInfo{
      .imageView = resourceManager->getImageView(DepthImageName),
      .imageLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal,
      .loadOp = vk::AttachmentLoadOp::eClear,
      .storeOp = vk::AttachmentStoreOp::eStore,
      .clearValue =
          vk::ClearValue{.depthStencil = vk::ClearDepthStencilValue{.depth = 1.f, .stencil = 0}},
  };

  const auto renderingInfo = vk::RenderingInfo{
      .renderArea = vk::Rect2D{.offset = {.x = 0, .y = 0},
                               .extent = resourceManager->getImageExtent(frame.getDrawImageId())},
      .layerCount = 1,
      .colorAttachmentCount = 1,
      .pColorAttachments = &colorAttachmentInfo,
      .pDepthAttachment = &depthAttachmentInfo};

  commandBuffer.begin(
      vk::CommandBufferBeginInfo{.flags = vk::CommandBufferUsageFlagBits::eSimultaneousUse});

  commandBuffer.beginRendering(renderingInfo);

  cubeRenderTask->record(commandBuffer);

  commandBuffer.endRendering();

  commandBuffer.end();
}

auto DefaultRenderScheduler::addStaticTask(const std::shared_ptr<IRenderTask> task) -> void {
  staticRenderTasks.push_back(task);
}

auto DefaultRenderScheduler::recordRenderTasks(Frame& frame) const -> void {

  {
    const auto& startCmd = frame.getCommandBuffer(CmdBufferType::Start);
    startCmd.begin(
        vk::CommandBufferBeginInfo{.flags = vk::CommandBufferUsageFlagBits::eSimultaneousUse});

    transitionImage(startCmd,
                    resourceManager->getImage(frame.getDrawImageId()),
                    vk::ImageLayout::eUndefined,
                    vk::ImageLayout::eColorAttachmentOptimal);

    startCmd.end();
  }

  executeStaticTasks(frame);

  const auto& endCmd = frame.getCommandBuffer(CmdBufferType::End);
  endCmd.begin(
      vk::CommandBufferBeginInfo{.flags = vk::CommandBufferUsageFlagBits::eSimultaneousUse});

  transitionImage(endCmd,
                  resourceManager->getImage(frame.getDrawImageId()),
                  vk::ImageLayout::eColorAttachmentOptimal,
                  vk::ImageLayout::eTransferSrcOptimal);

  transitionImage(endCmd,
                  swapchain->getSwapchainImage(frame.getSwapchainImageIndex()),
                  vk::ImageLayout::eUndefined,
                  vk::ImageLayout::eTransferDstOptimal);

  copyImageToImage(endCmd,
                   resourceManager->getImage(frame.getDrawImageId()),
                   swapchain->getSwapchainImage(frame.getSwapchainImageIndex()),
                   resourceManager->getImageExtent(frame.getDrawImageId()),
                   swapchain->getImageExtent());

  transitionImage(endCmd,
                  swapchain->getSwapchainImage(frame.getSwapchainImageIndex()),
                  vk::ImageLayout::eTransferDstOptimal,
                  vk::ImageLayout::ePresentSrcKHR);

  endCmd.end();
}

auto DefaultRenderScheduler::setupCommandBuffersForFrame(Frame& frame) -> void {
  auto staticCommandBuffer =
      commandBufferManager->getPrimaryCommandBuffer(frame.getIndex(), PoolId::Main);
  auto startCommandBuffer =
      commandBufferManager->getPrimaryCommandBuffer(frame.getIndex(), PoolId::Main);
  auto endCommandBuffer =
      commandBufferManager->getPrimaryCommandBuffer(frame.getIndex(), PoolId::Main);

  frame.clearCommandBuffers();

  frame.addCommandBuffer(CmdBufferType::Main, std::move(staticCommandBuffer));
  frame.addCommandBuffer(CmdBufferType::Start, std::move(startCommandBuffer));
  frame.addCommandBuffer(CmdBufferType::End, std::move(endCommandBuffer));
}

auto DefaultRenderScheduler::endFrame(Frame& frame) const -> void {
  // Get all the buffers one at a time because order matters
  const auto buffers = std::array{*frame.getCommandBuffer(CmdBufferType::Start),
                                  *frame.getCommandBuffer(CmdBufferType::Main),
                                  *frame.getCommandBuffer(CmdBufferType::End)};

  constexpr auto waitStages =
      std::array<vk::PipelineStageFlags, 1>{vk::PipelineStageFlagBits::eColorAttachmentOutput};
  const auto submitInfo = vk::SubmitInfo{
      .waitSemaphoreCount = 1,
      .pWaitSemaphores = &*frame.getImageAvailableSemaphore(),
      .pWaitDstStageMask = waitStages.data(),
      .commandBufferCount = static_cast<uint32_t>(buffers.size()),
      .pCommandBuffers = buffers.data(),
      .signalSemaphoreCount = 1,
      .pSignalSemaphores = &*frame.getRenderFinishedSemaphore(),
  };
  try {
    graphicsQueue->getQueue().submit(submitInfo, *frame.getInFlightFence());
  } catch (const std::exception& ex) {
    Log.error("Failed to submit command buffer submission {}", ex.what());
  }

  try {
    const auto swapchainImageIndex = frame.getSwapchainImageIndex();
    const auto chain = swapchain->getSwapchain();
    if (const auto result2 = graphicsQueue->getQueue().presentKHR(
            vk::PresentInfoKHR{.waitSemaphoreCount = 1,
                               .pWaitSemaphores = &*frame.getRenderFinishedSemaphore(),
                               .swapchainCount = 1,
                               .pSwapchains = &chain,
                               .pImageIndices = &swapchainImageIndex});
        result2 == vk::Result::eSuboptimalKHR) {
      Log.trace("Swapchain Needs Resized");
    }
  } catch (const std::exception& ex) { Log.trace("Swapchain needs recreated: {0}", ex.what()); }
}

auto DefaultRenderScheduler::transitionImage(const vk::raii::CommandBuffer& cmd,
                                             const vk::Image& image,
                                             vk::ImageLayout currentLayout,
                                             vk::ImageLayout newLayout) -> void {
  const auto barrier = vk::ImageMemoryBarrier{
      .srcAccessMask = vk::AccessFlagBits::eMemoryWrite,
      .dstAccessMask = vk::AccessFlagBits::eMemoryWrite | vk::AccessFlagBits::eMemoryRead,
      .oldLayout = currentLayout,
      .newLayout = newLayout,
      .image = image,
      .subresourceRange = {.aspectMask = vk::ImageAspectFlagBits::eColor,
                           .levelCount = 1,
                           .layerCount = 1}};

  cmd.pipelineBarrier(vk::PipelineStageFlagBits::eAllCommands,
                      vk::PipelineStageFlagBits::eAllCommands,
                      vk::DependencyFlagBits{}, // None
                      {},
                      {},
                      barrier);
}
auto DefaultRenderScheduler::copyImageToImage(const vk::raii::CommandBuffer& cmd,
                                              const vk::Image source,
                                              const vk::Image destination,
                                              const vk::Extent2D srcSize,
                                              const vk::Extent2D dstSize) -> void {
  const auto blitRegion = vk::ImageBlit2{
      .srcSubresource = {.aspectMask = vk::ImageAspectFlagBits::eColor, .layerCount = 1},
      .srcOffsets = std::array{vk::Offset3D{},
                               vk::Offset3D{.x = static_cast<int>(srcSize.width),
                                            .y = static_cast<int>(srcSize.height),
                                            .z = 1}},
      .dstSubresource = {.aspectMask = vk::ImageAspectFlagBits::eColor, .layerCount = 1},
      .dstOffsets = std::array{vk::Offset3D{},
                               vk::Offset3D{.x = static_cast<int>(dstSize.width),
                                            .y = static_cast<int>(dstSize.height),
                                            .z = 1}},
  };

  const auto blitInfo = vk::BlitImageInfo2{
      .srcImage = source,
      .srcImageLayout = vk::ImageLayout::eTransferSrcOptimal,
      .dstImage = destination,
      .dstImageLayout = vk::ImageLayout::eTransferDstOptimal,
      .regionCount = 1,
      .pRegions = &blitRegion,
      .filter = vk::Filter::eLinear,
  };

  cmd.blitImage2(blitInfo);
}
} // namespace tr

