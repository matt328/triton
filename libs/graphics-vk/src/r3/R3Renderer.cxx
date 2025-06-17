#include "R3Renderer.hpp"
#include "FrameState.hpp"
#include "buffers/BufferCreateInfo.hpp"
#include "buffers/BufferSystem.hpp"
#include "gfx/IFrameGraph.hpp"
#include "gfx/IFrameManager.hpp"
#include "gfx/QueueTypes.hpp"
#include "img/ImageManager.hpp"
#include "r3/GeometryBufferPack.hpp"
#include "r3/draw-context/ContextFactory.hpp"
#include "r3/draw-context/IDispatchContext.hpp"
#include "r3/graph/ResourceAliasRegistry.hpp"
#include "r3/render-pass/IRenderPass.hpp"
#include "r3/render-pass/RenderPassFactory.hpp"
#include "resources/allocators/GeometryAllocator.hpp"
#include "task/Frame.hpp"
#include "gfx/GeometryHandleMapper.hpp"
#include "ComponentIds.hpp"
#include "vk/command-buffer/CommandBufferManager.hpp"

namespace tr {

const std::unordered_map<ContextId, std::vector<PassId>> GraphicsMap = {
    {ContextId::Cube, {PassId::Forward}},
    {ContextId::Composition, {PassId::Composition}}};

const std::unordered_map<ContextId, std::vector<PassId>> ComputeMap = {
    {ContextId::Culling, {PassId::Culling}}};

R3Renderer::R3Renderer(RenderContextConfig newRenderConfig,
                       std::shared_ptr<IFrameManager> newFrameManager,
                       std::shared_ptr<queue::Graphics> newGraphicsQueue,
                       std::shared_ptr<Swapchain> newSwapchain,
                       std::shared_ptr<IFrameGraph> newFrameGraph,
                       std::shared_ptr<RenderPassFactory> newRenderPassFactory,
                       std::shared_ptr<CommandBufferManager> newCommandBufferManager,
                       std::shared_ptr<BufferSystem> newBufferSystem,
                       std::shared_ptr<ContextFactory> newDrawContextFactory,
                       std::shared_ptr<IStateBuffer> newStateBuffer,
                       std::shared_ptr<ImageManager> newImageManager,
                       std::shared_ptr<GeometryBufferPack> newGeometryBufferPack,
                       std::shared_ptr<FrameState> newFrameState,
                       std::shared_ptr<GeometryAllocator> newGeometryAllocator,
                       std::shared_ptr<GeometryHandleMapper> newGeometryHandleMapper,
                       std::shared_ptr<ResourceAliasRegistry> newAliasRegistry)
    : rendererConfig{newRenderConfig},
      frameManager{std::move(newFrameManager)},
      graphicsQueue{std::move(newGraphicsQueue)},
      swapchain{std::move(newSwapchain)},
      frameGraph{std::move(newFrameGraph)},
      renderPassFactory{std::move(newRenderPassFactory)},
      commandBufferManager{std::move(newCommandBufferManager)},
      bufferSystem{std::move(newBufferSystem)},
      drawContextFactory{std::move(newDrawContextFactory)},
      stateBuffer{std::move(newStateBuffer)},
      imageManager{std::move(newImageManager)},
      geometryBufferPack{std::move(newGeometryBufferPack)},
      frameState{std::move(newFrameState)},
      geometryAllocator{std::move(newGeometryAllocator)},
      geometryHandleMapper{std::move(newGeometryHandleMapper)},
      aliasRegistry{std::move(newAliasRegistry)} {
  Log.trace("Constructing R3Renderer");

  createGlobalBuffers();
  createGlobalImages();
  auto cullingPass = createComputeCullingPass();
  auto forwardPass = createForwardRenderPass();
  auto compositionPass = createCompositionRenderPass();

  frameGraph->addPass(std::move(cullingPass));
  frameGraph->addPass(std::move(forwardPass));
  frameGraph->addPass(std::move(compositionPass));

  const auto forwardDrawCreateInfo = ForwardDrawContextCreateInfo{
      .viewport = vk::Viewport{.width = static_cast<float>(rendererConfig.initialWidth),
                               .height = static_cast<float>(rendererConfig.initialHeight),
                               .minDepth = 0.f,
                               .maxDepth = 1.f},
      .scissor = vk::Rect2D{.offset = vk::Offset2D{.x = 0, .y = 0},
                            .extent = vk::Extent2D{.width = rendererConfig.initialWidth,
                                                   .height = rendererConfig.initialHeight}},
      .objectData = globalBuffers.objectData,
      .objectPositions = globalBuffers.objectPositions,
      .objectRotations = globalBuffers.objectRotations,
      .objectScales = globalBuffers.objectScales,
      .geometryRegion = globalBuffers.geometryRegion,
      .indexData = geometryBufferPack->getIndexBuffer(),
      .vertexPosition = geometryBufferPack->getPositionBuffer(),
      .vertexTexCoord = geometryBufferPack->getTexCoordBuffer(),
      .vertexColor = geometryBufferPack->getColorBuffer(),
      .vertexNormal = geometryBufferPack->getNormalBuffer(),
      .indirectCommand = globalBuffers.drawCommands,
      .indirectCount = globalBuffers.drawCounts,
  };

  drawContextFactory->createDispatchContext(ContextId::Cube, forwardDrawCreateInfo);

  const auto cullingCreateInfo =
      CullingDispatchContextCreateInfo{.objectData = globalBuffers.objectData,
                                       .geometryRegion = globalBuffers.geometryRegion,
                                       .objectPositions = globalBuffers.objectPositions,
                                       .objectRotations = globalBuffers.objectRotations,
                                       .objectScales = globalBuffers.objectScales,
                                       .indirectCommand = globalBuffers.drawCommands,
                                       .indirectCount = globalBuffers.drawCounts,
                                       .indexData = geometryBufferPack->getIndexBuffer(),
                                       .vertexPosition = geometryBufferPack->getPositionBuffer(),
                                       .vertexNormal = geometryBufferPack->getNormalBuffer(),
                                       .vertexTexCoord = geometryBufferPack->getTexCoordBuffer(),
                                       .vertexColor = geometryBufferPack->getColorBuffer()};

  drawContextFactory->createDispatchContext(ContextId::Culling, cullingCreateInfo);

  const auto compositionCreateInfo = CompositionContextCreateInfo{};

  drawContextFactory->createDispatchContext(ContextId::Composition, compositionCreateInfo);

  for (const auto& [contextId, passIds] : GraphicsMap) {
    for (const auto& passId : passIds) {
      frameGraph->getPass(passId)->registerDispatchContext(
          drawContextFactory->getDispatchContextHandle(contextId));
    }
  }

  for (const auto& [contextId, passIds] : ComputeMap) {
    for (const auto& passId : passIds) {
      frameGraph->getPass(passId)->registerDispatchContext(
          drawContextFactory->getDispatchContextHandle(contextId));
    }
  }

  frameGraph->bake();
}

auto R3Renderer::createGlobalBuffers() -> void {
  globalBuffers.drawCommands = bufferSystem->registerPerFrameBuffer(
      BufferCreateInfo{.bufferLifetime = BufferLifetime::Transient,
                       .initialSize = 20480,
                       .debugName = "DrawCommands",
                       .indirect = true});
  aliasRegistry->setHandle(BufferAlias::IndirectCommand, globalBuffers.drawCommands);

  globalBuffers.drawCounts = bufferSystem->registerPerFrameBuffer(
      BufferCreateInfo{.bufferLifetime = BufferLifetime::Transient,
                       .initialSize = 4,
                       .debugName = "DrawCounts",
                       .indirect = true});
  aliasRegistry->setHandle(BufferAlias::IndirectCommandCount, globalBuffers.drawCounts);

  globalBuffers.drawMetadata = bufferSystem->registerPerFrameBuffer(
      BufferCreateInfo{.bufferLifetime = BufferLifetime::Transient, .debugName = "DrawMetadata"});
  aliasRegistry->setHandle(BufferAlias::IndirectMetaData, globalBuffers.drawMetadata);

  globalBuffers.objectData = bufferSystem->registerPerFrameBuffer(
      BufferCreateInfo{.bufferLifetime = BufferLifetime::Transient,
                       .debugName = "Buffer-ObjectData"});
  aliasRegistry->setHandle(BufferAlias::ObjectData, globalBuffers.objectData);

  globalBuffers.objectPositions = bufferSystem->registerPerFrameBuffer(
      BufferCreateInfo{.bufferLifetime = BufferLifetime::Transient,
                       .debugName = "Buffer-ObjectPositions"});
  aliasRegistry->setHandle(BufferAlias::ObjectPositions, globalBuffers.objectPositions);

  globalBuffers.objectRotations = bufferSystem->registerPerFrameBuffer(
      BufferCreateInfo{.bufferLifetime = BufferLifetime::Transient,
                       .debugName = "Buffer-ObjectRotations"});
  aliasRegistry->setHandle(BufferAlias::ObjectRotations, globalBuffers.objectRotations);

  globalBuffers.objectScales = bufferSystem->registerPerFrameBuffer(
      BufferCreateInfo{.bufferLifetime = BufferLifetime::Transient,
                       .debugName = "Buffer-ObjectScales"});
  aliasRegistry->setHandle(BufferAlias::ObjectScales, globalBuffers.objectScales);

  globalBuffers.geometryRegion = bufferSystem->registerPerFrameBuffer(
      BufferCreateInfo{.bufferLifetime = BufferLifetime::Transient,
                       .debugName = "Buffer-GeometryRegion"});
  aliasRegistry->setHandle(BufferAlias::GeometryRegion, globalBuffers.geometryRegion);
}

auto R3Renderer::createGlobalImages() -> void {

  globalImages.forwardColorImage = imageManager->createPerFrameImage(ImageRequest{
      .logicalName = "forward",
      .format = vk::Format::eR16G16B16A16Sfloat,
      .extent = vk::Extent2D{.width = rendererConfig.initialWidth,
                             .height = rendererConfig.initialHeight},
      .usageFlags = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled,
      .aspectFlags = vk::ImageAspectFlagBits::eColor});

  // TODO(renderer-1): fix depth image, handle non-per frame images
  globalImages.forwardDepthImage = imageManager->createPerFrameImage(
      ImageRequest{.logicalName = "depth",
                   .format = vk::Format::eD32Sfloat,
                   .extent = vk::Extent2D{.width = rendererConfig.initialWidth,
                                          .height = rendererConfig.initialHeight},
                   .usageFlags = vk::ImageUsageFlagBits::eDepthStencilAttachment,
                   .aspectFlags = vk::ImageAspectFlagBits::eDepth});

  aliasRegistry->setHandle(ImageAlias::SwapchainImage, imageManager->getSwapchainImageHandle());

  aliasRegistry->setHandle(ImageAlias::GeometryColorImage, globalImages.forwardColorImage);
  aliasRegistry->setHandle(ImageAlias::DepthImage, globalImages.forwardDepthImage);
}

void R3Renderer::renderNextFrame() {
  ZoneScopedN("renderNextFrame");
  const auto result = frameManager->acquireFrame();

  if (std::holds_alternative<ImageAcquireResult>(result)) {
    const auto acquireResult = std::get<ImageAcquireResult>(result);
    if (acquireResult == ImageAcquireResult::Error) {
      Log.warn("Failed to acquire swapchain image");
      return;
    }
    Log.warn("Some other Result returned from acquireFrame");
    return;
  }

  auto* frame = std::get<Frame*>(result);

  std::optional<std::pair<SimState, SimState>> states = std::nullopt;
  {
    ZoneScopedN("getStates");
    Timestamp currentTime = std::chrono::steady_clock::now();
    int retries = 100; // e.g. timeout after ~100 * 10ms = 1s
    while (retries-- > 0 && states == std::nullopt) {
      ZoneScopedN("getStates try");
      states = stateBuffer->getStates(currentTime);
      if (states == std::nullopt) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
      }
    }
  }

  if (states != std::nullopt) {
    auto& current = (*states).first;

    buildFrameState(current.objectMetadata, current.stateHandles, geometryRegionContents);

    {
      ZoneScopedN("Per Frame buffers");
      // GeometryRegionBuffer
      if (!geometryRegionContents.empty()) {
        bufferSystem->insert(
            frame->getLogicalBuffer(globalBuffers.geometryRegion),
            geometryRegionContents.data(),
            BufferRegion{.size = sizeof(GpuGeometryRegionData) * geometryRegionContents.size()});
      }

      // Object Data Buffers
      if (!current.objectMetadata.empty()) {
        bufferSystem->insert(
            frame->getLogicalBuffer(globalBuffers.objectData),
            current.objectMetadata.data(),
            BufferRegion{.size = sizeof(GpuObjectData) * current.objectMetadata.size()});
      }
      if (!current.positions.empty()) {
        bufferSystem->insert(
            frame->getLogicalBuffer(globalBuffers.objectPositions),
            current.positions.data(),
            BufferRegion{.size = sizeof(GpuTransformData) * current.positions.size()});
      }
      if (!current.rotations.empty()) {
        bufferSystem->insert(
            frame->getLogicalBuffer(globalBuffers.objectRotations),
            current.rotations.data(),
            BufferRegion{.size = sizeof(GpuRotationData) * current.rotations.size()});
      }
      if (!current.scales.empty()) {
        bufferSystem->insert(frame->getLogicalBuffer(globalBuffers.objectScales),
                             current.scales.data(),
                             BufferRegion{.size = sizeof(GpuScaleData) * current.scales.size()});
      }
      // Set host values in frame
      frame->setObjectCount(current.objectMetadata.size());
    }
  } else {
    Log.warn(
        "Failed to get states this frame. Either game world is behind, or we're shutting down");
  }

  const auto& results = frameGraph->execute(frame);
  endFrame(frame, results);
}

auto R3Renderer::buildFrameState(std::vector<GpuObjectData>& objectData,
                                 std::vector<StateHandles>& stateHandles,
                                 std::vector<GpuGeometryRegionData>& regionBuffer) -> void {
  ZoneScopedN("R3Renderer::buildFrameState");
  regionBuffer.clear();
  regionBuffer.reserve(objectData.size());
  for (size_t i = 0; i < objectData.size(); ++i) {
    auto regionHandle = geometryHandleMapper->toInternal(stateHandles[i].geometryHandle);
    auto regionData = geometryAllocator->getRegionData(*regionHandle);
    objectData[i].geometryRegionId = regionBuffer.size();
    regionBuffer.push_back(regionData);
  }
}

auto R3Renderer::endFrame(const Frame* frame, const FrameGraphResult& results) -> void {
  ZoneScopedN("R3Renderer::endFrame");
  buffers.clear();

  constexpr auto waitStages =
      std::array<vk::PipelineStageFlags, 1>{vk::PipelineStageFlagBits::eColorAttachmentOutput};

  const auto submitInfo = vk::SubmitInfo{
      .waitSemaphoreCount = 1,
      .pWaitSemaphores = &*frame->getImageAvailableSemaphore(),
      .pWaitDstStageMask = waitStages.data(),
      .commandBufferCount = static_cast<uint32_t>(results.commandBuffers.size()),
      .pCommandBuffers = results.commandBuffers.data(),
      .signalSemaphoreCount = 1,
      .pSignalSemaphores = &*frame->getRenderFinishedSemaphore(),
  };

  try {
    ZoneScopedN("queue submit");
    const auto fence = *frame->getInFlightFence();
    graphicsQueue->getQueue().submit(submitInfo, fence);
    frameState->advanceFrame();
  } catch (const std::exception& ex) {
    Log.error("Failed to submit command buffer submission {}", ex.what());
  }

  try {
    ZoneScopedN("queue present");
    const auto swapchainImageIndex = frame->getSwapchainImageIndex();
    const auto chain = swapchain->getSwapchain();

    const auto presentInfo =
        vk::PresentInfoKHR{.waitSemaphoreCount = 1,
                           .pWaitSemaphores = &*frame->getRenderFinishedSemaphore(),
                           .swapchainCount = 1,
                           .pSwapchains = &chain,
                           .pImageIndices = &swapchainImageIndex};

    if (const auto result2 = graphicsQueue->getQueue().presentKHR(presentInfo);
        result2 == vk::Result::eSuboptimalKHR || result2 == vk::Result::eErrorOutOfDateKHR) {
      Log.trace("Swapchain Needs Resized");
    }
    FrameMark;
  } catch (const std::exception& ex) { Log.trace("Swapchain needs recreated: {0}", ex.what()); }
}

void R3Renderer::waitIdle() {
}

auto R3Renderer::createComputeCullingPass() -> std::unique_ptr<IRenderPass> {

  auto cullingPass = renderPassFactory->createRenderPass(RenderPassCreateInfo{
      .passId = PassId::Culling,
      .passInfo = CullingPassCreateInfo{},
  });

  auto cmdBufferUses = std::vector<CommandBufferUse>{};

  for (const auto& frame : frameManager->getFrames()) {
    cmdBufferUses.emplace_back(CommandBufferUse{.threadId = std::this_thread::get_id(),
                                                .frameId = frame->getIndex(),
                                                .passId = cullingPass->getId()});
  }

  const auto commandBufferInfo = CommandBufferInfo{
      .queueConfigs = {QueueConfig{.queueType = QueueType::Graphics, .uses = cmdBufferUses}}};
  commandBufferManager->allocateCommandBuffers(commandBufferInfo);

  return cullingPass;
}

auto R3Renderer::createForwardRenderPass() -> std::unique_ptr<IRenderPass> {
  auto forwardPass = renderPassFactory->createRenderPass(RenderPassCreateInfo{
      .passId = PassId::Forward,
      .passInfo = ForwardPassCreateInfo{.colorImage = ImageAlias::GeometryColorImage,
                                        .depthImage = ImageAlias::DepthImage}});

  std::vector<CommandBufferUse> cmdBufferUses{};

  for (const auto& frame : frameManager->getFrames()) {
    cmdBufferUses.emplace_back(CommandBufferUse{.threadId = std::this_thread::get_id(),
                                                .frameId = frame->getIndex(),
                                                .passId = forwardPass->getId()});
  }

  const auto commandBufferInfo = CommandBufferInfo{
      .queueConfigs = {QueueConfig{.queueType = QueueType::Graphics, .uses = cmdBufferUses}}};

  commandBufferManager->allocateCommandBuffers(commandBufferInfo);

  return forwardPass;
}

auto R3Renderer::createCompositionRenderPass() -> std::unique_ptr<IRenderPass> {
  auto compositionPass = renderPassFactory->createRenderPass(RenderPassCreateInfo{
      .passId = PassId::Composition,
      .passInfo = CompositionPassCreateInfo{.colorImage = ImageAlias::GeometryColorImage,
                                            .swapchainImage = ImageAlias::SwapchainImage},
  });

  std::vector<CommandBufferUse> cmdBufferUses{};

  for (const auto& frame : frameManager->getFrames()) {
    cmdBufferUses.emplace_back(CommandBufferUse{.threadId = std::this_thread::get_id(),
                                                .frameId = frame->getIndex(),
                                                .passId = compositionPass->getId()});
  }

  const auto commandBufferInfo = CommandBufferInfo{
      .queueConfigs = {QueueConfig{.queueType = QueueType::Graphics, .uses = cmdBufferUses}}};

  commandBufferManager->allocateCommandBuffers(commandBufferInfo);

  return compositionPass;
}
}
