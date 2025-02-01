#include "DefaultRenderScheduler.hpp"

#include "cm/Handles.hpp"
#include "tr/Events.hpp"
#include "tr/IEventBus.hpp"
#include "tr/IGuiSystem.hpp"
#include "vk/BufferManager.hpp"
#include "vk/CommandBufferManager.hpp"
#include "Maths.hpp"
#include "gfx/QueueTypes.hpp"
#include "cm/GpuObjectData.hpp"
#include "task/Frame.hpp"
#include "task/IRenderTask.hpp"
#include "vk/VkResourceManager.hpp"

namespace tr {

DefaultRenderScheduler::DefaultRenderScheduler(
    std::shared_ptr<IFrameManager> newFrameManager,
    std::shared_ptr<CommandBufferManager> newCommandBufferManager,
    std::shared_ptr<queue::Graphics> newGraphicsQueue,
    std::shared_ptr<VkResourceManager> newResourceManager,
    std::shared_ptr<Swapchain> newSwapchain,
    std::shared_ptr<RenderTaskFactory> newRenderTaskFactory,
    std::shared_ptr<IGuiSystem> newGuiSystem,
    std::shared_ptr<BufferManager> newBufferManager,
    const RenderContextConfig& rendererConfig,
    const std::shared_ptr<IEventBus>& eventBus)
    : frameManager{std::move(newFrameManager)},
      commandBufferManager{std::move(newCommandBufferManager)},
      graphicsQueue{std::move(newGraphicsQueue)},
      resourceManager{std::move(newResourceManager)},
      swapchain{std::move(newSwapchain)},
      renderTaskFactory{std::move(newRenderTaskFactory)},
      guiSystem{std::move(newGuiSystem)},
      bufferManager{std::move(newBufferManager)},
      renderConfig{rendererConfig} {

  eventBus->subscribe<SwapchainResized>(
      [&](const SwapchainResized& event) { handleSwapchainResized(event); });

  buffers.resize(3);

  const auto drawImageExtent = vk::Extent2D{
      .width = maths::scaleNumber(swapchain->getImageExtent().width, renderConfig.renderScale),
      .height = maths::scaleNumber(swapchain->getImageExtent().height, renderConfig.renderScale)};

  const auto depthImageHandle =
      resourceManager->createDepthImageAndView(DepthImageName,
                                               drawImageExtent,
                                               swapchain->getDepthFormat());

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

    createStaticBuffers(frame);
    createSkinnedBuffers(frame);

    // Camera Data Buffer
    {
      const auto name = frame->getIndexedName("Buffer-CameraData-Frame_");
      const auto handle = bufferManager->createBuffer(
          sizeof(GpuCameraData),
          vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eShaderDeviceAddress,
          name);

      auto& cameraDataBuffer = bufferManager->getBuffer(handle);

      cameraDataBuffer.mapBuffer();
      cameraDataBuffer.updateBufferValue(&cameraData, sizeof(GpuCameraData));
      cameraDataBuffer.unmapBuffer();
      frame->setBufferHandle(BufferHandleType::CameraBuffer, handle);
    }

    frame->setupRenderingInfo(resourceManager);
  }

  resourceManager->createComputePipeline("Pipeline-Compute");

  indirectRenderTask = renderTaskFactory->createIndirectRenderTask();
  computeTask = renderTaskFactory->createComputeTask();
  staticRenderTask = renderTaskFactory->createStaticTask();

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

auto DefaultRenderScheduler::createStaticBuffers(const std::unique_ptr<Frame>& frame) -> void {
  // Gpu Buffer Entry Data
  {
    std::vector<GpuBufferEntry> gpuBufferEntryList{};
    gpuBufferEntryList.reserve(1);
    const auto name = frame->getIndexedName("Buffer-StaticGpuBufferEntry-Frame_");
    const auto handle = bufferManager->createBuffer(
        sizeof(GpuBufferEntry) * renderConfig.maxStaticObjects,
        vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eShaderDeviceAddress,
        name);

    frame->setBufferHandle(BufferHandleType::StaticGpuBufferEntry, handle);

    auto& gpuBufferEntriesBuffer = bufferManager->getBuffer(handle);
    gpuBufferEntriesBuffer.mapBuffer();
    gpuBufferEntriesBuffer.updateBufferValue(gpuBufferEntryList.data(), sizeof(GpuBufferEntry));
    gpuBufferEntriesBuffer.unmapBuffer();
  }

  // IndirectCommandBuffer
  {
    const auto name = frame->getIndexedName("Buffer-DrawCommand-Frame_");
    const auto handle = bufferManager->createBuffer(
        sizeof(vk::DrawIndexedIndirectCommand) * renderConfig.maxStaticObjects,
        vk::BufferUsageFlagBits::eIndirectBuffer | vk::BufferUsageFlagBits::eShaderDeviceAddress,
        name);
    frame->setBufferHandle(BufferHandleType::StaticDrawCommand, handle);
    auto& indirectCommandBuffer = bufferManager->getBuffer(handle);
    auto cmd = vk::DrawIndexedIndirectCommand{};
    cmd.instanceCount = 0;
    indirectCommandBuffer.mapBuffer();
    indirectCommandBuffer.updateBufferValue(&cmd, sizeof(vk::DrawIndexedIndirectCommand));
    indirectCommandBuffer.unmapBuffer();
  }

  // Object Count Buffer
  {
    const auto name = frame->getIndexedName("Buffer-Count-Frame_");
    const auto handle = bufferManager->createBuffer(
        sizeof(uint32_t),
        vk::BufferUsageFlagBits::eIndirectBuffer | vk::BufferUsageFlagBits::eShaderDeviceAddress,
        name);
    auto& countBuffer = bufferManager->getBuffer(handle);

    uint32_t count = 0;
    countBuffer.mapBuffer();
    countBuffer.updateBufferValue(&count, sizeof(uint32_t));
    countBuffer.unmapBuffer();

    frame->setBufferHandle(BufferHandleType::StaticCountBuffer, handle);
  }

  // ObjectDataIndex Buffer
  {
    const auto name = frame->getIndexedName("Buffer-ObjectDataIndex-Frame_");
    const auto handle = bufferManager->createBuffer(
        sizeof(uint32_t) * renderConfig.maxStaticObjects,
        vk::BufferUsageFlagBits::eIndirectBuffer | vk::BufferUsageFlagBits::eShaderDeviceAddress,
        name);
    auto& objectDataIndexBuffer = bufferManager->getBuffer(handle);

    uint32_t count = 0;
    objectDataIndexBuffer.mapBuffer();
    objectDataIndexBuffer.updateBufferValue(&count, sizeof(uint32_t));
    objectDataIndexBuffer.unmapBuffer();
    frame->setBufferHandle(BufferHandleType::StaticObjectDataIndexBuffer, handle);
  }

  // Object Data Buffer
  {
    const auto name = frame->getIndexedName("Buffer-GpuObjectData-Frame_");
    const auto handle = bufferManager->createBuffer(
        sizeof(GpuObjectData) * renderConfig.maxStaticObjects,
        vk::BufferUsageFlagBits::eIndirectBuffer | vk::BufferUsageFlagBits::eShaderDeviceAddress,
        name);
    frame->setBufferHandle(BufferHandleType::StaticObjectDataBuffer, handle);
  }
}

auto DefaultRenderScheduler::createSkinnedBuffers(const std::unique_ptr<Frame>& frame) -> void {
  // Gpu Buffer Entry Data - for Compute Shader to create draw commands
  {
    std::vector<GpuBufferEntry> gpuBufferEntryList{};
    gpuBufferEntryList.reserve(1);
    const auto name = frame->getIndexedName("Buffer-SkinnedGpuBufferEntry-Frame_");
    const auto handle = bufferManager->createBuffer(
        sizeof(GpuBufferEntry) * renderConfig.maxDynamicObjects,
        vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eShaderDeviceAddress,
        name);

    frame->setBufferHandle(BufferHandleType::DynamicGpuBufferEntry, handle);

    auto& gpuBufferEntriesBuffer = bufferManager->getBuffer(handle);
    gpuBufferEntriesBuffer.mapBuffer();
    gpuBufferEntriesBuffer.updateBufferValue(gpuBufferEntryList.data(), sizeof(GpuBufferEntry));
    gpuBufferEntriesBuffer.unmapBuffer();
  }

  // IndirectCommandBuffer
  {
    const auto name = frame->getIndexedName("Buffer-DynamicDrawCommand-Frame_");
    const auto handle = bufferManager->createBuffer(
        sizeof(vk::DrawIndexedIndirectCommand) * renderConfig.maxDynamicObjects,
        vk::BufferUsageFlagBits::eIndirectBuffer | vk::BufferUsageFlagBits::eShaderDeviceAddress,
        name);
    frame->setBufferHandle(BufferHandleType::DynamicDrawCommand, handle);
    auto& indirectCommandBuffer = bufferManager->getBuffer(handle);
    auto cmd = vk::DrawIndexedIndirectCommand{};
    cmd.instanceCount = 0;
    indirectCommandBuffer.mapBuffer();
    indirectCommandBuffer.updateBufferValue(&cmd, sizeof(vk::DrawIndexedIndirectCommand));
    indirectCommandBuffer.unmapBuffer();
  }

  // Object Count Buffer
  {
    const auto name = frame->getIndexedName("Buffer-DynamicCount-Frame_");
    const auto handle = bufferManager->createBuffer(
        sizeof(uint32_t),
        vk::BufferUsageFlagBits::eIndirectBuffer | vk::BufferUsageFlagBits::eShaderDeviceAddress,
        name);
    auto& countBuffer = bufferManager->getBuffer(handle);

    uint32_t count = 0;
    countBuffer.mapBuffer();
    countBuffer.updateBufferValue(&count, sizeof(uint32_t));
    countBuffer.unmapBuffer();

    frame->setBufferHandle(BufferHandleType::DynamicCountBuffer, handle);
  }

  // ObjectDataIndex Buffer
  {
    const auto name = frame->getIndexedName("Buffer-DynamicObjectDataIndex-Frame_");
    const auto handle = bufferManager->createBuffer(
        sizeof(uint32_t) * renderConfig.maxDynamicObjects,
        vk::BufferUsageFlagBits::eIndirectBuffer | vk::BufferUsageFlagBits::eShaderDeviceAddress,
        name);
    auto& objectDataIndexBuffer = bufferManager->getBuffer(handle);

    uint32_t count = 0;
    objectDataIndexBuffer.mapBuffer();
    objectDataIndexBuffer.updateBufferValue(&count, sizeof(uint32_t));
    objectDataIndexBuffer.unmapBuffer();
    frame->setBufferHandle(BufferHandleType::DynamicObjectDataIndexBuffer, handle);
  }

  // Object Data Buffer
  {
    const auto name = frame->getIndexedName("Buffer-DynamicObjectData-Frame_");
    const auto handle = bufferManager->createBuffer(
        sizeof(GpuObjectData) * renderConfig.maxDynamicObjects,
        vk::BufferUsageFlagBits::eIndirectBuffer | vk::BufferUsageFlagBits::eShaderDeviceAddress,
        name);
    frame->setBufferHandle(BufferHandleType::DynamicObjectDataBuffer, handle);
  }

  // Animation Data Buffer
  {
    const auto name = frame->getIndexedName("Buffer-AnimationData-Frame_");
    const auto bufferSize = sizeof(AnimationData) * renderConfig.maxDynamicObjects;
    const auto flags =
        vk::BufferUsageFlagBits::eIndirectBuffer | vk::BufferUsageFlagBits::eShaderDeviceAddress;
    const auto handle = bufferManager->createBuffer(bufferSize, flags, name);
    frame->setBufferHandle(BufferHandleType::AnimationDataBuffer, handle);
  }
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

  frame.setStaticObjectCount(renderData.staticGpuMeshData.size());
  frame.setSkinnedObjectCount(renderData.skinnedMeshData.size());

  resourceManager->updateShaderBindings();

  updateStaticBuffers(frame, renderData);

  updateDynamicBuffers(frame, renderData);

  // Update CameraDataBuffer
  {
    auto& cameraDataBuffer =
        bufferManager->getBuffer(frame.getBufferHandle(BufferHandleType::CameraBuffer));
    cameraDataBuffer.mapBuffer();
    cameraDataBuffer.updateBufferValue(&renderData.cameraData, sizeof(GpuCameraData));
    cameraDataBuffer.unmapBuffer();
  }
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

  {
    ZoneNamedN(var, "RenderTasks", true);
    executeTasks(frame, recordTasks);
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

auto DefaultRenderScheduler::executeTasks(Frame& frame, bool recordTasks) const -> void {

  auto& commandBuffer = commandBufferManager->getCommandBuffer(frame.getMainCommandBufferHandle());

  commandBuffer.begin(
      vk::CommandBufferBeginInfo{.flags = vk::CommandBufferUsageFlagBits::eSimultaneousUse});

  {
    ZoneNamedN(var, "Record Static Compute Task", true);
    const auto& gpuBufferEntryBuffer =
        bufferManager->getBuffer(frame.getBufferHandle(BufferHandleType::StaticGpuBufferEntry));
    const auto& objectDataBuffer =
        bufferManager->getBuffer(frame.getBufferHandle(BufferHandleType::StaticObjectDataBuffer));
    const auto& drawCommandBuffer =
        bufferManager->getBuffer(frame.getBufferHandle(BufferHandleType::StaticDrawCommand));
    const auto& countBuffer =
        bufferManager->getBuffer(frame.getBufferHandle(BufferHandleType::StaticCountBuffer));
    const auto& objectDataIndexBuffer = bufferManager->getBuffer(
        frame.getBufferHandle(BufferHandleType::StaticObjectDataIndexBuffer));

    auto computePushConstants = ComputePushConstants{
        .drawCommandBufferAddress = drawCommandBuffer.getDeviceAddress(),
        .gpuBufferEntryBufferAddress = gpuBufferEntryBuffer.getDeviceAddress(),
        .objectDataBufferAddress = objectDataBuffer.getDeviceAddress(),
        .countBufferAddress = countBuffer.getDeviceAddress(),
        .objectDataIndexBufferAddress = objectDataIndexBuffer.getDeviceAddress(),
        .objectCount = 2};
    computeTask->record(commandBuffer, computePushConstants);
  }

  {
    ZoneNamedN(var, "Record Dynamic Compute Task", true);
    const auto& gpuBufferEntryBuffer =
        bufferManager->getBuffer(frame.getBufferHandle(BufferHandleType::DynamicGpuBufferEntry));
    const auto& objectDataBuffer =
        bufferManager->getBuffer(frame.getBufferHandle(BufferHandleType::DynamicObjectDataBuffer));
    const auto& drawCommandBuffer =
        bufferManager->getBuffer(frame.getBufferHandle(BufferHandleType::DynamicDrawCommand));
    const auto& countBuffer =
        bufferManager->getBuffer(frame.getBufferHandle(BufferHandleType::DynamicCountBuffer));
    const auto& objectDataIndexBuffer = bufferManager->getBuffer(
        frame.getBufferHandle(BufferHandleType::DynamicObjectDataIndexBuffer));

    auto computePushConstants = ComputePushConstants{
        .drawCommandBufferAddress = drawCommandBuffer.getDeviceAddress(),
        .gpuBufferEntryBufferAddress = gpuBufferEntryBuffer.getDeviceAddress(),
        .objectDataBufferAddress = objectDataBuffer.getDeviceAddress(),
        .countBufferAddress = countBuffer.getDeviceAddress(),
        .objectDataIndexBufferAddress = objectDataIndexBuffer.getDeviceAddress(),
        .objectCount = 1};
    computeTask->record(commandBuffer, computePushConstants);
  }

  // Record barriers for buffers written by Static compute tasks
  {
    auto& indirectBuffer =
        bufferManager->getBuffer(frame.getBufferHandle(BufferHandleType::StaticDrawCommand));
    insertBarrier(commandBuffer, indirectBuffer);

    auto& countBuffer =
        bufferManager->getBuffer(frame.getBufferHandle(BufferHandleType::StaticCountBuffer));
    insertBarrier(commandBuffer, countBuffer);

    auto& objectDataIndexBuffer = bufferManager->getBuffer(
        frame.getBufferHandle(BufferHandleType::StaticObjectDataIndexBuffer));
    insertBarrier(commandBuffer, objectDataIndexBuffer);
  }

  // Record barriers for buffers written by Dynamic compute tasks
  {
    auto& indirectBuffer =
        bufferManager->getBuffer(frame.getBufferHandle(BufferHandleType::DynamicDrawCommand));
    insertBarrier(commandBuffer, indirectBuffer);

    auto& countBuffer =
        bufferManager->getBuffer(frame.getBufferHandle(BufferHandleType::DynamicCountBuffer));
    insertBarrier(commandBuffer, countBuffer);

    auto& objectDataIndexBuffer = bufferManager->getBuffer(
        frame.getBufferHandle(BufferHandleType::DynamicObjectDataIndexBuffer));
    insertBarrier(commandBuffer, objectDataIndexBuffer);
  }

  const auto renderingInfo = frame.getRenderingInfo();

  commandBuffer.beginRendering(renderingInfo);

  commandBuffer.setViewportWithCount({viewport});
  commandBuffer.setScissorWithCount({snezzor});

  if (recordTasks) {
    ZoneNamedN(var, "IndirectRenderTask", true);
    if (frame.getStaticObjectCount() > 0) {
      staticRenderTask->record(commandBuffer, frame);
    }
    if (frame.getSkinnedObjectCount() > 0) {
      indirectRenderTask->record(commandBuffer, frame);
    }
  }

  commandBuffer.endRendering();

  commandBuffer.end();
}

auto DefaultRenderScheduler::endFrame(Frame& frame) -> void {

  buffers.clear();
  buffers.push_back(*commandBufferManager->getCommandBuffer(frame.getStartCommandBufferHandle()));
  buffers.push_back(*commandBufferManager->getCommandBuffer(frame.getMainCommandBufferHandle()));
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

auto DefaultRenderScheduler::insertBarrier(const vk::raii::CommandBuffer& cmd, const Buffer& buffer)
    -> void {
  // Insert a memory barrier for the buffer the computeTask writes to
  vk::BufferMemoryBarrier bufferMemoryBarrier{
      .srcAccessMask = vk::AccessFlagBits::eShaderWrite,
      .dstAccessMask = vk::AccessFlagBits::eIndirectCommandRead,
      .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
      .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
      .buffer = buffer.getBuffer(),
      .offset = 0,
      .size = VK_WHOLE_SIZE,
  };

  cmd.pipelineBarrier(vk::PipelineStageFlagBits::eComputeShader,
                      vk::PipelineStageFlagBits::eDrawIndirect,
                      vk::DependencyFlags{},
                      nullptr,
                      bufferMemoryBarrier,
                      nullptr);
}

auto DefaultRenderScheduler::updateStaticBuffers(Frame& frame, const RenderData& renderData)
    -> void {
  // Update GpuBufferEntriesBuffer
  {
    ZoneNamedN(var, "getStaticGpuData", true);
    const auto& gpuBufferEntryList =
        resourceManager->getStaticGpuData(renderData.staticGpuMeshData);

    auto& gpuBufferEntriesBuffer =
        bufferManager->getBuffer(frame.getBufferHandle(BufferHandleType::StaticGpuBufferEntry));

    gpuBufferEntriesBuffer.mapBuffer();
    gpuBufferEntriesBuffer.updateBufferValue(gpuBufferEntryList.data(),
                                             sizeof(GpuBufferEntry) * gpuBufferEntryList.size());
    gpuBufferEntriesBuffer.unmapBuffer();
  }

  // Update StaticObjectDataBuffer
  {
    auto& objectDataBuffer =
        bufferManager->getBuffer(frame.getBufferHandle(BufferHandleType::StaticObjectDataBuffer));
    objectDataBuffer.mapBuffer();
    objectDataBuffer.updateBufferValue(renderData.objectData.data(),
                                       sizeof(GpuObjectData) * renderData.objectData.size());
    objectDataBuffer.unmapBuffer();
  }
}

auto DefaultRenderScheduler::updateDynamicBuffers(Frame& frame, const RenderData& renderData)
    -> void {
  // Update GpuBufferEntriesBuffer
  {
    ZoneNamedN(var, "getDynamicGpuData", true);
    const auto& gpuBufferEntryList = resourceManager->getSkinnedGpuData(renderData.skinnedMeshData);

    auto& gpuBufferEntriesBuffer =
        bufferManager->getBuffer(frame.getBufferHandle(BufferHandleType::DynamicGpuBufferEntry));

    gpuBufferEntriesBuffer.mapBuffer();
    gpuBufferEntriesBuffer.updateBufferValue(gpuBufferEntryList.data(),
                                             sizeof(GpuBufferEntry) * gpuBufferEntryList.size());
    gpuBufferEntriesBuffer.unmapBuffer();
  }

  // Update ObjectDataBuffer
  {
    const auto handle = frame.getBufferHandle(BufferHandleType::DynamicObjectDataBuffer);
    auto& objectDataBuffer = bufferManager->getBuffer(handle);
    const auto size =
        sizeof(renderData.dynamicObjectData.front()) * renderData.dynamicObjectData.size();

    objectDataBuffer.mapBuffer();
    objectDataBuffer.updateBufferValue(renderData.dynamicObjectData.data(), size);
    objectDataBuffer.unmapBuffer();
  }

  // Update AnimationDataBuffer
  {
    const auto handle = frame.getBufferHandle(BufferHandleType::AnimationDataBuffer);
    auto& animationDataBuffer = bufferManager->getBuffer(handle);
    const auto size = sizeof(renderData.animationData.front()) * renderData.animationData.size();

    animationDataBuffer.mapBuffer();
    animationDataBuffer.updateBufferValue(renderData.animationData.data(), size);
    animationDataBuffer.unmapBuffer();
  }
}

} // namespace tr
