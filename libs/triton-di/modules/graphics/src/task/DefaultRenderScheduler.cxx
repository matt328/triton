#include "DefaultRenderScheduler.hpp"

#include "tr/Events.hpp"
#include "tr/IEventBus.hpp"
#include "tr/IGuiSystem.hpp"
#include "vk/CommandBufferManager.hpp"
#include "Maths.hpp"
#include "gfx/QueueTypes.hpp"
#include "cm/GpuObjectData.hpp"
#include "task/Frame.hpp"
#include "task/IRenderTask.hpp"
#include "vk/MeshBufferManager.hpp"
#include <vulkan/vulkan_enums.hpp>

namespace tr {

DefaultRenderScheduler::DefaultRenderScheduler(
    std::shared_ptr<IFrameManager> newFrameManager,
    std::shared_ptr<CommandBufferManager> newCommandBufferManager,
    std::shared_ptr<queue::Graphics> newGraphicsQueue,
    std::shared_ptr<VkResourceManager> newResourceManager,
    std::shared_ptr<Swapchain> newSwapchain,
    std::shared_ptr<RenderTaskFactory> newRenderTaskFactory,
    std::shared_ptr<IGuiSystem> newGuiSystem,
    const RenderContextConfig& rendererConfig,
    const std::shared_ptr<IEventBus>& eventBus)
    : frameManager{std::move(newFrameManager)},
      commandBufferManager{std::move(newCommandBufferManager)},
      graphicsQueue{std::move(newGraphicsQueue)},
      resourceManager{std::move(newResourceManager)},
      swapchain{std::move(newSwapchain)},
      renderTaskFactory{std::move(newRenderTaskFactory)},
      guiSystem{std::move(newGuiSystem)},
      renderConfig{rendererConfig} {

  eventBus->subscribe<SwapchainResized>(
      [&](const SwapchainResized& event) { handleSwapchainResized(event); });

  buffers.resize(3);

  const auto drawImageExtent = vk::Extent2D{
      .width = maths::scaleNumber(swapchain->getImageExtent().width, renderConfig.renderScale),
      .height = maths::scaleNumber(swapchain->getImageExtent().height, renderConfig.renderScale)};

  auto depthImageHandle = resourceManager->createDepthImageAndView(DepthImageName,
                                                                   drawImageExtent,
                                                                   swapchain->getDepthFormat());

  std::vector<GpuBufferEntry> gpuBufferEntryList{};
  gpuBufferEntryList.reserve(1);

  const auto position = glm::vec3{0.f, 0.f, 0.f};
  const auto view = glm::lookAt(position, glm::vec3{0.f, 0.f, -5.f}, glm::vec3{0.f, 1.f, 0.f});
  const auto projection =
      glm::perspective(glm::radians(60.f), static_cast<float>(1920 / 1080), 0.1f, 10000.0f);

  const auto cameraData = GpuCameraData{.view = view,
                                        .proj = projection,
                                        .viewProj = view * projection,
                                        .position = glm::vec4{0.f, 0.f, 0.f, 1.f}};

  for (const auto& frame : frameManager->getFrames()) {

    frame->setDepthImageHandle(depthImageHandle);

    // Gpu Buffer Entry Data
    {
      const auto name = frame->getIndexedName("Buffer-GpuBufferEntry-Frame_");
      const auto handle = resourceManager->createBuffer(
          sizeof(GpuBufferEntry) * renderConfig.maxStaticObjects,
          vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eShaderDeviceAddress,
          name);

      frame->setGpuBufferEntryBufferHandle(handle);

      auto& gpuBufferEntriesBuffer = resourceManager->getBuffer(handle);
      gpuBufferEntriesBuffer.mapBuffer();
      gpuBufferEntriesBuffer.updateBufferValue(gpuBufferEntryList.data(), sizeof(GpuBufferEntry));
      gpuBufferEntriesBuffer.unmapBuffer();
    }

    // IndirectCommandBuffer
    {
      const auto name = frame->getIndexedName("Buffer-DrawCommand-Frame_");
      const auto handle = resourceManager->createBuffer(
          sizeof(vk::DrawIndexedIndirectCommand) *
              renderConfig.maxStaticObjects, // Size this buffer to max_objects?
          vk::BufferUsageFlagBits::eIndirectBuffer | vk::BufferUsageFlagBits::eShaderDeviceAddress,
          name);
      frame->setDrawCommandBufferHandle(handle);
      auto& indirectCommandBuffer = resourceManager->getBuffer(handle);
      auto cmd = vk::DrawIndexedIndirectCommand{};
      cmd.instanceCount = 0;
      indirectCommandBuffer.mapBuffer();
      indirectCommandBuffer.updateBufferValue(&cmd, sizeof(vk::DrawIndexedIndirectCommand));
      indirectCommandBuffer.unmapBuffer();
    }

    // GpuObjectDataBuffer
    {
      const auto name = frame->getIndexedName("Buffer-GpuObjectData-Frame_");
      const auto handle = resourceManager->createBuffer(
          sizeof(GpuObjectData) * renderConfig.maxStaticObjects,
          vk::BufferUsageFlagBits::eIndirectBuffer | vk::BufferUsageFlagBits::eShaderDeviceAddress,
          name);
      frame->setGpuObjectDataBufferHandle(handle);
    }

    // Camera Data Buffer
    {
      const auto name = frame->getIndexedName("Buffer-CameraData-Frame_");
      const auto handle = resourceManager->createBuffer(
          sizeof(GpuCameraData),
          vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eShaderDeviceAddress,
          name);

      auto& cameraDataBuffer = resourceManager->getBuffer(handle);

      cameraDataBuffer.mapBuffer();
      cameraDataBuffer.updateBufferValue(&cameraData, sizeof(GpuCameraData));
      cameraDataBuffer.unmapBuffer();

      frame->setCameraBufferHandle(handle);
    }

    // Object Count Buffer
    {
      const auto name = frame->getIndexedName("Buffer-Count-Frame_");
      const auto handle = resourceManager->createBuffer(
          sizeof(uint32_t),
          vk::BufferUsageFlagBits::eIndirectBuffer | vk::BufferUsageFlagBits::eShaderDeviceAddress,
          name);
      auto& countBuffer = resourceManager->getBuffer(handle);

      uint32_t count = 0;
      countBuffer.mapBuffer();
      countBuffer.updateBufferValue(&count, sizeof(uint32_t));
      countBuffer.unmapBuffer();

      frame->setCountBufferHandle(handle);
    }

    // Descriptor Buffer (Textures)
    {
      const auto bufferSize = 24;

      const auto flags = vk::BufferUsageFlagBits::eResourceDescriptorBufferEXT |
                         vk::BufferUsageFlagBits::eSamplerDescriptorBufferEXT |
                         vk::BufferUsageFlagBits::eShaderDeviceAddress |
                         vk::BufferUsageFlagBits::eTransferDst;

      const auto name = frame->getIndexedName("Buffer-Descriptor-Frame_");

      [[maybe_unused]] const auto handle = resourceManager->createBuffer(bufferSize, flags, name);
    }

    frame->setupRenderingInfo(resourceManager);
  }

  resourceManager->createComputePipeline("Pipeline-Compute");

  indirectRenderTask = renderTaskFactory->createIndirectRenderTask();
  computeTask = renderTaskFactory->createComputeTask();

  const auto extent = swapchain->getImageExtent();

  viewport = vk::Viewport{
      .width = static_cast<float>(extent.width),
      .height = static_cast<float>(extent.height),
      .minDepth = 0.f,
      .maxDepth = 1.f,
  };

  snezzor = vk::Rect2D{.offset = vk::Offset2D{.x = 0, .y = 0}, .extent = extent};
}

DefaultRenderScheduler::~DefaultRenderScheduler() {
  Log.trace("Destroying DefaultRenderScheduler");
}

auto DefaultRenderScheduler::handleSwapchainResized(const SwapchainResized& event) -> void {
  viewport = vk::Viewport{
      .width = static_cast<float>(event.width),
      .height = static_cast<float>(event.height),
      .minDepth = 0.f,
      .maxDepth = 1.f,
  };

  snezzor = vk::Rect2D{.offset = vk::Offset2D{.x = 0, .y = 0},
                       .extent = vk::Extent2D{.width = event.width, .height = event.height}};

  const auto drawImageExtent =
      vk::Extent2D{.width = maths::scaleNumber(event.width, renderConfig.renderScale),
                   .height = maths::scaleNumber(event.height, renderConfig.renderScale)};

  // All frames use the same depth image so just get the .front here
  resourceManager->destroyImage(frameManager->getFrames().front()->getDepthImageHandle());

  auto depthImageHandle = resourceManager->createDepthImageAndView(DepthImageName,
                                                                   drawImageExtent,
                                                                   swapchain->getDepthFormat());
  for (const auto& frame : frameManager->getFrames()) {
    frame->setDepthImageHandle(depthImageHandle);
    frame->setupRenderingInfo(resourceManager);
  }
}

auto DefaultRenderScheduler::updatePerFrameRenderData(Frame& frame, const RenderData& renderData)
    -> void {
  ZoneNamedN(var, "updatePerFrameRenderData", true);

  { // Update GpuBufferEntriesBuffer
    ZoneNamedN(var, "getStaticGpuData", true);
    const auto& gpuBufferEntryList =
        resourceManager->getStaticGpuData(renderData.staticGpuMeshData);

    auto& gpuBufferEntriesBuffer =
        resourceManager->getBuffer(frame.getGpuBufferEntryBufferHandle());

    gpuBufferEntriesBuffer.mapBuffer();
    {
      ZoneNamedN(var, "updateBufferValue", true);

      gpuBufferEntriesBuffer.updateBufferValue(gpuBufferEntryList.data(),
                                               sizeof(GpuBufferEntry) * gpuBufferEntryList.size());
    }
    gpuBufferEntriesBuffer.unmapBuffer();
  }

  // Update GpuObjectDataBuffer
  auto& objectDataBuffer = resourceManager->getBuffer(frame.getGpuObjectDataBufferHandle());
  objectDataBuffer.mapBuffer();
  objectDataBuffer.updateBufferValue(renderData.objectData.data(),
                                     sizeof(GpuObjectData) * renderData.objectData.size());
  objectDataBuffer.unmapBuffer();

  // Update CameraDataBuffer
  auto& cameraDataBuffer = resourceManager->getBuffer(frame.getCameraBufferHandle());
  cameraDataBuffer.mapBuffer();
  cameraDataBuffer.updateBufferValue(&renderData.cameraData, sizeof(GpuCameraData));
  cameraDataBuffer.unmapBuffer();
}

auto DefaultRenderScheduler::recordRenderTasks(Frame& frame, bool recordTasks) -> void {
  tasksRecorded = false;
  ZoneNamedN(var, "Record Command Buffers", true);
  {
    ZoneNamedN(var, "Start", true);
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

  if (recordTasks) {
    ZoneNamedN(var, "RenderTasks", true);
    executeTasks(frame);
    tasksRecorded = true;
  }

  {
    ZoneNamedN(var, "End", true);
    auto& endCmd = commandBufferManager->getCommandBuffer(frame.getEndCommandBufferHandle());
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

    guiSystem->render(endCmd,
                      swapchain->getSwapchainImageView(frame.getSwapchainImageIndex()),
                      swapchain->getImageExtent());

    transitionImage(endCmd,
                    swapchain->getSwapchainImage(frame.getSwapchainImageIndex()),
                    vk::ImageLayout::eTransferDstOptimal,
                    vk::ImageLayout::ePresentSrcKHR);

    endCmd.end();
  }
}

auto DefaultRenderScheduler::executeTasks(Frame& frame) const -> void {

  auto& commandBuffer = commandBufferManager->getCommandBuffer(frame.getMainCommandBufferHandle());

  commandBuffer.begin(
      vk::CommandBufferBeginInfo{.flags = vk::CommandBufferUsageFlagBits::eSimultaneousUse});

  {
    ZoneNamedN(var, "ComputeTask", true);
    computeTask->record(commandBuffer, frame);
  }

  {
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
  }

  {
    auto& countBuffer = resourceManager->getBuffer(frame.getCountBufferHandle());
    // Insert a memory barrier for the buffer the computeTask writes to
    vk::BufferMemoryBarrier countBufferBarrier{
        .srcAccessMask = vk::AccessFlagBits::eShaderWrite,
        .dstAccessMask = vk::AccessFlagBits::eIndirectCommandRead,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .buffer = countBuffer.getBuffer(),
        .offset = 0,
        .size = VK_WHOLE_SIZE,
    };

    commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eComputeShader,
                                  vk::PipelineStageFlagBits::eDrawIndirect,
                                  vk::DependencyFlags{},
                                  nullptr,
                                  countBufferBarrier,
                                  nullptr);
  }

  const auto renderingInfo = frame.getRenderingInfo();

  commandBuffer.beginRendering(renderingInfo);

  commandBuffer.setViewportWithCount({viewport});
  commandBuffer.setScissorWithCount({snezzor});

  {
    ZoneNamedN(var, "IndirectRenderTask", true);
    indirectRenderTask->record(commandBuffer, frame);
  }

  commandBuffer.endRendering();

  commandBuffer.end();
}

auto DefaultRenderScheduler::endFrame(Frame& frame) -> void {

  buffers.clear();
  buffers.push_back(*commandBufferManager->getCommandBuffer(frame.getStartCommandBufferHandle()));

  if (tasksRecorded) {
    buffers.push_back(*commandBufferManager->getCommandBuffer(frame.getMainCommandBufferHandle()));
  }

  buffers.push_back(*commandBufferManager->getCommandBuffer(frame.getEndCommandBufferHandle()));

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

    const auto presentInfo =
        vk::PresentInfoKHR{.waitSemaphoreCount = 1,
                           .pWaitSemaphores = &*frame.getRenderFinishedSemaphore(),
                           .swapchainCount = 1,
                           .pSwapchains = &chain,
                           .pImageIndices = &swapchainImageIndex};

    if (const auto result2 = graphicsQueue->getQueue().presentKHR(presentInfo);
        result2 == vk::Result::eSuboptimalKHR || result2 == vk::Result::eErrorOutOfDateKHR) {
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
