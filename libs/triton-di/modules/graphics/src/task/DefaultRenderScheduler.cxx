#include "DefaultRenderScheduler.hpp"
#include "vk/CommandBufferManager.hpp"
#include "Maths.hpp"
#include "gfx/QueueTypes.hpp"
#include "cm/ObjectData.hpp"
#include "task/Frame.hpp"
#include "task/IRenderTask.hpp"
#include "task/PoolId.hpp"
#include "task/graph/TaskGraph.hpp"
#include <vulkan/vulkan_raii.hpp>

namespace tr {

struct InstanceData {
  glm::mat4 model;
  uint32_t indexCount;
  uint32_t instanceCount;
  uint32_t firstIndex;
  uint32_t instanceID;
};

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

  const auto drawImageExtent = vk::Extent2D{
      .width = maths::scaleNumber(swapchain->getImageExtent().width, rendererConfig.renderScale),
      .height = maths::scaleNumber(swapchain->getImageExtent().height, rendererConfig.renderScale)};

  auto depthImageHandle = resourceManager->createDepthImageAndView(DepthImageName,
                                                                   drawImageExtent,
                                                                   swapchain->getDepthFormat());

  const auto instanceData =
      InstanceData{.model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -5.0f)),
                   .indexCount = 36,
                   .instanceCount = 1,
                   .firstIndex = 0,
                   .instanceID = 0};

  const auto position = glm::vec3{0.f, 0.f, 0.f};
  const auto view = glm::lookAt(position, glm::vec3{0.f, 0.f, -5.f}, glm::vec3{0.f, 1.f, 0.f});
  const auto projection =
      glm::perspective(glm::radians(60.f), static_cast<float>(1920 / 1080), 0.1f, 10000.0f);

  const auto cameraData = CameraData{.view = view,
                                     .proj = projection,
                                     .viewProj = view * projection,
                                     .position = glm::vec4{0.f, 0.f, 0.f, 1.f}};

  for (const auto& frame : frameManager->getFrames()) {

    frame->setDepthImageHandle(depthImageHandle);

    // Instance Data Buffer
    {
      const auto name = frame->getIndexedName("Buffer-InstanceData-Frame_");
      const auto handle = resourceManager->createBuffer(
          sizeof(InstanceData),
          vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eShaderDeviceAddress,
          name);
      auto& instanceBuffer = resourceManager->getBuffer(handle);
      frame->setInstanceDataBufferHandle(handle);

      instanceBuffer.mapBuffer();
      instanceBuffer.updateBufferValue(&instanceData, sizeof(InstanceData));
      instanceBuffer.unmapBuffer();
    }

    // IndirectCommandBuffer
    { // TODO(matt) Come up with more solid buffer referencing than just hard coded strings
      const auto name = frame->getIndexedName("Buffer-DrawCommand-Frame_");
      const auto handle = resourceManager->createBuffer(
          sizeof(vk::DrawIndexedIndirectCommand),
          vk::BufferUsageFlagBits::eIndirectBuffer | vk::BufferUsageFlagBits::eShaderDeviceAddress,
          name);
      frame->setDrawCommandBufferHandle(handle);
    }

    // Camera Data Buffer
    {
      const auto name = frame->getIndexedName("Buffer-CameraData-Frame_");
      const auto handle = resourceManager->createBuffer(
          sizeof(CameraData),
          vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eShaderDeviceAddress,
          name);

      auto& cameraDataBuffer = resourceManager->getBuffer(handle);

      cameraDataBuffer.mapBuffer();
      cameraDataBuffer.updateBufferValue(&cameraData, sizeof(CameraData));
      cameraDataBuffer.unmapBuffer();

      frame->setCameraBufferHandle(handle);
    }
    frame->setupRenderingInfo(resourceManager);
  }

  resourceManager->createComputePipeline("Pipeline-Compute");

  cubeRenderTask = renderTaskFactory->createCubeRenderTask();
  computeTask = renderTaskFactory->createComputeTask();

  viewport = vk::Viewport{
      .width = 1920,
      .height = 1080,
      .minDepth = 0.f,
      .maxDepth = 1.f,
  };

  snezzor = vk::Rect2D{.offset = vk::Offset2D{.x = 0, .y = 0},
                       .extent = vk::Extent2D{.width = 1920, .height = 1080}};
}

DefaultRenderScheduler::~DefaultRenderScheduler() {
  Log.trace("Destroying DefaultRenderScheduler");
}

auto DefaultRenderScheduler::executeTasks(Frame& frame) const -> void {

  auto& commandBuffer = commandBufferManager->getCommandBuffer(frame.getMainCommandBufferHandle());

  commandBuffer.begin(
      vk::CommandBufferBeginInfo{.flags = vk::CommandBufferUsageFlagBits::eSimultaneousUse});

  {
    ZoneNamedN(var, "ComputeTask", true);
    computeTask->record(commandBuffer, frame);
  }

  auto& indirectBuffer = resourceManager->getBuffer(frame.getDrawCommandBufferHandle());

  // Insert a memory barrier for the buffer the computeTask writes to
  vk::BufferMemoryBarrier bufferMemoryBarrier{
      .srcAccessMask = vk::AccessFlagBits::eShaderWrite,
      .dstAccessMask = vk::AccessFlagBits::eIndirectCommandRead,
      .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
      .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
      .buffer = indirectBuffer.getBuffer(),
      .offset = 0,
      .size = VK_WHOLE_SIZE,
  };

  commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eComputeShader,
                                vk::PipelineStageFlagBits::eDrawIndirect,
                                vk::DependencyFlags{},
                                nullptr,
                                bufferMemoryBarrier,
                                nullptr);

  const auto renderingInfo = frame.getRenderingInfo();

  commandBuffer.beginRendering(renderingInfo);

  commandBuffer.setViewportWithCount({viewport});
  commandBuffer.setScissorWithCount({snezzor});

  {
    ZoneNamedN(var, "CubeRenderTask", true);
    cubeRenderTask->record(commandBuffer, frame);
  }

  commandBuffer.endRendering();

  commandBuffer.end();
}

auto DefaultRenderScheduler::recordRenderTasks(Frame& frame) const -> void {

  {
    ZoneNamedN(var, "StartCmd", true);
    const auto& startCmd =
        commandBufferManager->getCommandBuffer(frame.getStartCommandBufferHandle());
    startCmd.begin(
        vk::CommandBufferBeginInfo{.flags = vk::CommandBufferUsageFlagBits::eSimultaneousUse});

    transitionImage(startCmd,
                    resourceManager->getImage(frame.getDrawImageHandle()),
                    vk::ImageLayout::eUndefined,
                    vk::ImageLayout::eColorAttachmentOptimal);

    startCmd.end();
  }

  {
    ZoneNamedN(var, "ExecuteTasks", true);
    executeTasks(frame);
  }

  {
    ZoneNamedN(var, "EndCmd", true);
    const auto& endCmd = commandBufferManager->getCommandBuffer(frame.getEndCommandBufferHandle());
    endCmd.begin(
        vk::CommandBufferBeginInfo{.flags = vk::CommandBufferUsageFlagBits::eSimultaneousUse});

    transitionImage(endCmd,
                    resourceManager->getImage(frame.getDrawImageHandle()),
                    vk::ImageLayout::eColorAttachmentOptimal,
                    vk::ImageLayout::eTransferSrcOptimal);

    transitionImage(endCmd,
                    swapchain->getSwapchainImage(frame.getSwapchainImageIndex()),
                    vk::ImageLayout::eUndefined,
                    vk::ImageLayout::eTransferDstOptimal);

    copyImageToImage(endCmd,
                     resourceManager->getImage(frame.getDrawImageHandle()),
                     swapchain->getSwapchainImage(frame.getSwapchainImageIndex()),
                     resourceManager->getImageExtent(frame.getDrawImageHandle()),
                     swapchain->getImageExtent());

    transitionImage(endCmd,
                    swapchain->getSwapchainImage(frame.getSwapchainImageIndex()),
                    vk::ImageLayout::eTransferDstOptimal,
                    vk::ImageLayout::ePresentSrcKHR);

    endCmd.end();
  }
}

auto DefaultRenderScheduler::endFrame(Frame& frame) const -> void {

  // Get all the buffers one at a time because order matters
  const auto buffers = std::array<vk::CommandBuffer, 3>{
      *commandBufferManager->getCommandBuffer(frame.getStartCommandBufferHandle()),
      *commandBufferManager->getCommandBuffer(frame.getMainCommandBufferHandle()),
      *commandBufferManager->getCommandBuffer(frame.getEndCommandBufferHandle()),
  };

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
