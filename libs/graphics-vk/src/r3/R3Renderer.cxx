#include "R3Renderer.hpp"
#include "FrameState.hpp"
#include "api/gw/EditorStateBuffer.hpp"
#include "buffers/BufferCreateInfo.hpp"
#include "buffers/BufferSystem.hpp"
#include "gfx/IFrameGraph.hpp"
#include "gfx/IFrameManager.hpp"
#include "gfx/QueueTypes.hpp"
#include "img/ImageManager.hpp"
#include "img/TextureArena.hpp"
#include "r3/GeometryBufferPack.hpp"
#include "r3/draw-context/ContextFactory.hpp"
#include "r3/draw-context/IDispatchContext.hpp"
#include "r3/graph/ResourceAliasRegistry.hpp"
#include "r3/render-pass/IRenderPass.hpp"
#include "r3/render-pass/RenderPassFactory.hpp"
#include "r3/render-pass/passes/PresentPass.hpp"
#include "resources/allocators/GeometryAllocator.hpp"
#include "task/Frame.hpp"
#include "gfx/HandleMapperTypes.hpp"
#include "ComponentIds.hpp"
#include "vk/command-buffer/CommandBufferManager.hpp"
#include "vk/sb/DSLayoutManager.hpp"
#include "vk/sb/IShaderBinding.hpp"
#include "vk/sb/IShaderBindingFactory.hpp"
#include "api/GlmToString.hpp"

namespace tr {

const std::unordered_map<ContextId, std::vector<PassId>> GraphicsMap = {
    {ContextId::Cube, {PassId::Forward}},
    {ContextId::ImGui, {PassId::ImGui}},
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
                       std::shared_ptr<ResourceAliasRegistry> newAliasRegistry,
                       std::shared_ptr<IShaderBindingFactory> newShaderBindingFactory,
                       std::shared_ptr<DSLayoutManager> newLayoutManager,
                       std::shared_ptr<EditorStateBuffer> newEditorStateBuffer,
                       std::shared_ptr<ImageTransitionQueue> newImageQueue,
                       std::shared_ptr<TextureHandleMapper> newTextureHandleMapper,
                       std::shared_ptr<TextureArena> newTextureArena)
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
      aliasRegistry{std::move(newAliasRegistry)},
      shaderBindingFactory{std::move(newShaderBindingFactory)},
      layoutManager{std::move(newLayoutManager)},
      editorStateBuffer{std::move(newEditorStateBuffer)},
      imageQueue{std::move(newImageQueue)},
      textureHandleMapper{std::move(newTextureHandleMapper)},
      textureArena{std::move(newTextureArena)} {
  Log.trace("Constructing R3Renderer");

  createGlobalBuffers();
  createGlobalImages();
  createGlobalShaderBindings();

  auto cullingPass = createComputeCullingPass();
  auto forwardPass = createForwardRenderPass();
  auto imguiPass = createImGuiPass();
  auto compositionPass = createCompositionRenderPass();
  auto presentPass = createPresentPass();

  frameGraph->addPass(std::move(cullingPass));
  frameGraph->addPass(std::move(forwardPass));
  frameGraph->addPass(std::move(imguiPass));
  frameGraph->addPass(std::move(compositionPass));
  frameGraph->addPass(std::move(presentPass));

  const auto forwardDrawCreateInfo = ForwardDrawContextCreateInfo{
      .viewport = vk::Viewport{.width = static_cast<float>(rendererConfig.initialWidth),
                               .height = static_cast<float>(rendererConfig.initialHeight),
                               .minDepth = 0.f,
                               .maxDepth = 1.f},
      .scissor = vk::Rect2D{.offset = vk::Offset2D{.x = 0, .y = 0},
                            .extent = vk::Extent2D{.width = rendererConfig.initialWidth,
                                                   .height = rendererConfig.initialHeight}},
      .resourceTable = globalBuffers.resourceTable,
      .frameData = globalBuffers.frameData,
      .indirectCommand = globalBuffers.drawCommands,
      .indirectCommandCount = globalBuffers.drawCounts,
  };

  drawContextFactory->createDispatchContext(ContextId::Cube, forwardDrawCreateInfo);

  const auto cullingCreateInfo = CullingDispatchContextCreateInfo{
      .resourceTable = globalBuffers.resourceTable,
      .frameData = globalBuffers.frameData,
  };

  drawContextFactory->createDispatchContext(ContextId::Culling, cullingCreateInfo);

  const auto compositionCreateInfo = CompositionContextCreateInfo{
      .viewport = vk::Viewport{.width = static_cast<float>(rendererConfig.initialWidth),
                               .height = static_cast<float>(rendererConfig.initialHeight),
                               .minDepth = 0.f,
                               .maxDepth = 1.f},
      .scissor = vk::Rect2D{.offset = vk::Offset2D{.x = 0, .y = 0},
                            .extent = vk::Extent2D{.width = rendererConfig.initialWidth,
                                                   .height = rendererConfig.initialHeight}},
      .defaultShaderBinding = globalShaderBindings.defaultBinding};

  drawContextFactory->createDispatchContext(ContextId::Composition, compositionCreateInfo);

  const auto imguiCreateInfo = ImGuiContextCreateInfo{
      .viewport = vk::Viewport{.width = static_cast<float>(rendererConfig.initialWidth),
                               .height = static_cast<float>(rendererConfig.initialHeight),
                               .minDepth = 0.f,
                               .maxDepth = 1.f},
      .scissor = vk::Rect2D{.offset = vk::Offset2D{.x = 0, .y = 0},
                            .extent = vk::Extent2D{.width = rendererConfig.initialWidth,
                                                   .height = rendererConfig.initialHeight}},
  };
  drawContextFactory->createDispatchContext(ContextId::ImGui, imguiCreateInfo);

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
                       .initialSize = 40960,
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
                       .initialSize = 8192,
                       .debugName = "Buffer-GeometryRegion"});
  aliasRegistry->setHandle(BufferAlias::GeometryRegion, globalBuffers.geometryRegion);

  globalBuffers.materials = bufferSystem->registerPerFrameBuffer(
      {.bufferLifetime = BufferLifetime::Transient, .debugName = "Buffer-Materials"});
  aliasRegistry->setHandle(BufferAlias::Materials, globalBuffers.materials);

  globalBuffers.frameData = bufferSystem->registerPerFrameBuffer(BufferCreateInfo{
      .bufferLifetime = BufferLifetime::Transient,
      .bufferUsage = BufferUsage::Storage,
      .debugName = "Buffer-FrameData",
  });
  aliasRegistry->setHandle(BufferAlias::FrameData, globalBuffers.frameData);

  globalBuffers.resourceTable = bufferSystem->registerPerFrameBuffer(BufferCreateInfo{
      .bufferLifetime = BufferLifetime::Transient,
      .bufferUsage = BufferUsage::Storage,
      .debugName = "Buffer-ResourceTable",
  });
  aliasRegistry->setHandle(BufferAlias::ResourceTable, globalBuffers.resourceTable);
}

auto R3Renderer::createGlobalImages() -> void {

  globalImages.forwardColorImage = imageManager->createPerFrameImage(ImageRequest{
      .logicalName = "forward",
      .format = vk::Format::eR16G16B16A16Sfloat,
      .extent = vk::Extent2D{.width = rendererConfig.initialWidth,
                             .height = rendererConfig.initialHeight},
      .usageFlags = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled,
      .aspectFlags = vk::ImageAspectFlagBits::eColor,
      .debugName = "forward"});

  globalImages.imguiColorImage = imageManager->createPerFrameImage(ImageRequest{
      .logicalName = "imgui",
      .format = vk::Format::eR16G16B16A16Sfloat,
      .extent = vk::Extent2D{.width = rendererConfig.initialWidth,
                             .height = rendererConfig.initialHeight},
      .usageFlags = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled,
      .aspectFlags = vk::ImageAspectFlagBits::eColor,
      .debugName = "imgui"});

  // TODO(renderer-1): fix depth image, handle non-per frame images
  globalImages.forwardDepthImage = imageManager->createPerFrameImage(
      ImageRequest{.logicalName = "depth",
                   .format = vk::Format::eD32Sfloat,
                   .extent = vk::Extent2D{.width = rendererConfig.initialWidth,
                                          .height = rendererConfig.initialHeight},
                   .usageFlags = vk::ImageUsageFlagBits::eDepthStencilAttachment,
                   .aspectFlags = vk::ImageAspectFlagBits::eDepth,
                   .debugName = "depth"});

  aliasRegistry->setHandle(ImageAlias::SwapchainImage, imageManager->getSwapchainImageHandle());

  aliasRegistry->setHandle(ImageAlias::GeometryColorImage, globalImages.forwardColorImage);
  aliasRegistry->setHandle(ImageAlias::DepthImage, globalImages.forwardDepthImage);
  aliasRegistry->setHandle(ImageAlias::GuiColorImage, globalImages.imguiColorImage);
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
  std::optional<EditorState> editorState = std::nullopt;
  {
    ZoneScopedN("getStates");
    Timestamp currentTime = Clock::now();
    int retries = 100; // e.g. timeout after ~100 * 10ms = 1s
    while (retries-- > 0 && states == std::nullopt) {
      ZoneScopedN("getStates try");
      states = stateBuffer->getStates(currentTime);
      if (states == std::nullopt) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
      }
      editorState = editorStateBuffer->getStates(currentTime);
      if (editorState == std::nullopt) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
      }
    }
  }

  if (editorState) {
    frame->setEditorState(editorState);
  }

  if (states != std::nullopt) {
    auto& current = (*states).first;

    textureArena->updateShaderBindings(frame);
    buildFrameState(current.objectMetadata, current.stateHandles, geometryRegionContents);

    {
      ZoneScopedN("Per Frame buffers");
      // FrameData
      auto frameData = GpuFrameData{.view = current.view,
                                    .projection = current.projection,
                                    .cameraPosition = glm::vec4(0.f, 0.f, 5.f, 1.f),
                                    .time = 0.f,
                                    .maxObjects = frame->getObjectCount()};

      bufferSystem->insert(frame->getLogicalBuffer(globalBuffers.frameData),
                           &frameData,
                           BufferRegion{.size = sizeof(GpuFrameData)});

      // ResourceTable
      auto resourceTableData = GpuResourceTable{
          .objectDataBufferAddress =
              bufferSystem->getBufferAddress(frame->getLogicalBuffer(globalBuffers.objectData))
                  .or_else([] {
                    Log.warn("getBufferAddress could not find an address for objectData buffer");
                    return std::optional<uint64_t>{0};
                  })
                  .value(),
          .objectPositionsAddress =
              bufferSystem->getBufferAddress(frame->getLogicalBuffer(globalBuffers.objectPositions))
                  .value_or(0L),
          .objectRotationsAddress =
              bufferSystem->getBufferAddress(frame->getLogicalBuffer(globalBuffers.objectRotations))
                  .value_or(0L),
          .objectScalesAddress =
              bufferSystem->getBufferAddress(frame->getLogicalBuffer(globalBuffers.objectScales))
                  .value_or(0L),
          .regionBufferAddress =
              bufferSystem->getBufferAddress(frame->getLogicalBuffer(globalBuffers.geometryRegion))
                  .value_or(0L),
          .indexBufferAddress =
              bufferSystem->getBufferAddress(geometryBufferPack->getIndexBuffer()).value_or(0L),
          .positionBufferAddress =
              bufferSystem->getBufferAddress(geometryBufferPack->getPositionBuffer()).value_or(0L),
          .colorBufferAddress =
              bufferSystem->getBufferAddress(geometryBufferPack->getColorBuffer()).value_or(0L),
          .texCoordBufferAddress =
              bufferSystem->getBufferAddress(geometryBufferPack->getTexCoordBuffer()).value_or(0L),
          .normalBufferAddress =
              bufferSystem->getBufferAddress(geometryBufferPack->getNormalBuffer()).value_or(0L),
          .materialBufferAddress =
              bufferSystem->getBufferAddress(frame->getLogicalBuffer(globalBuffers.materials))
                  .value_or(0L),
          .indirectCommandAddress =
              bufferSystem->getBufferAddress(frame->getLogicalBuffer(globalBuffers.drawCommands))
                  .value_or(0L),
          .indirectCountAddress =
              bufferSystem->getBufferAddress(frame->getLogicalBuffer(globalBuffers.drawCounts))
                  .value_or(0L),
      };
      bufferSystem->insert(frame->getLogicalBuffer(globalBuffers.resourceTable),
                           &resourceTableData,
                           BufferRegion{.size = sizeof(GpuResourceTable)});

      // GeometryRegionBuffer
      if (!geometryRegionContents.empty()) {
        bufferSystem->insert(
            frame->getLogicalBuffer(globalBuffers.geometryRegion),
            geometryRegionContents.data(),
            BufferRegion{.size = sizeof(GpuGeometryRegionData) * geometryRegionContents.size()});
      }

      // MaterialDataBuffer
      if (!materialDataContents.empty()) {
        bufferSystem->insert(
            frame->getLogicalBuffer(globalBuffers.materials),
            materialDataContents.data(),
            BufferRegion{.size = sizeof(GpuMaterialData) * materialDataContents.size()});
      }
      // TODO: might need to allow resizing transient buffers, or at least track their size/usage as
      // metrics and get a better sense of how large they need to be. Right now the materialData
      // buffer is not large enough to hold 60 materials
      //  Object Data Buffers
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
      // For some reason the last object's scales are getting set to 0s
      static_assert(sizeof(GpuScaleData) == 12);
      if (!current.scales.empty()) {
        bufferSystem->insert(frame->getLogicalBuffer(globalBuffers.objectScales),
                             current.scales.data(),
                             BufferRegion{.size = sizeof(GpuScaleData) * current.scales.size()});
      }
      // Set host values in frame
      frame->setImageTransitionInfo(imageQueue->dequeue());
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

  materialDataContents.clear();

  for (size_t i = 0; i < objectData.size(); ++i) {
    auto regionHandle = geometryHandleMapper->toInternal(stateHandles[i].geometryHandle);
    auto regionData = geometryAllocator->getRegionData(*regionHandle);
    objectData[i].geometryRegionId = regionBuffer.size();
    if (stateHandles[i].textureHandle) {
      auto textureHandle = textureHandleMapper->toInternal(*stateHandles[i].textureHandle);
      auto textureId = textureArena->getTextureIndex(*textureHandle);
      materialDataContents.push_back(
          GpuMaterialData{.baseColor = {1.f, 0.f, 0.f, 1.f}, .albedoTextureId = textureId});
    }
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
                                        .depthImage = ImageAlias::DepthImage,
                                        .dsLayoutHandles = {textureArena->getDSLayoutHandle()}}});

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
      .passInfo =
          CompositionPassCreateInfo{.colorImage = ImageAlias::GeometryColorImage,
                                    .swapchainImage = ImageAlias::SwapchainImage,
                                    .defaultShaderBinding = globalShaderBindings.defaultBinding,
                                    .defaultDSLayout = globalShaderBindings.defaultBindingLayout},
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

auto R3Renderer::createImGuiPass() -> std::unique_ptr<IRenderPass> {
  auto imGuiPass = renderPassFactory->createRenderPass(RenderPassCreateInfo{
      .passId = PassId::ImGui,
      .passInfo = ImGuiPassCreateInfo{.colorImage = ImageAlias::GuiColorImage}});

  std::vector<CommandBufferUse> cmdBufferUses{};

  for (const auto& frame : frameManager->getFrames()) {
    cmdBufferUses.emplace_back(CommandBufferUse{.threadId = std::this_thread::get_id(),
                                                .frameId = frame->getIndex(),
                                                .passId = imGuiPass->getId()});
  }

  const auto commandBufferInfo = CommandBufferInfo{
      .queueConfigs = {QueueConfig{.queueType = QueueType::Graphics, .uses = cmdBufferUses}}};

  commandBufferManager->allocateCommandBuffers(commandBufferInfo);

  return imGuiPass;
}

auto R3Renderer::createPresentPass() -> std::unique_ptr<IRenderPass> {
  auto pass = std::make_unique<PresentPass>(PassId::Present);
  std::vector<CommandBufferUse> cmdBufferUses{};

  for (const auto& frame : frameManager->getFrames()) {
    cmdBufferUses.emplace_back(CommandBufferUse{.threadId = std::this_thread::get_id(),
                                                .frameId = frame->getIndex(),
                                                .passId = pass->getId()});
  }

  const auto commandBufferInfo = CommandBufferInfo{
      .queueConfigs = {QueueConfig{.queueType = QueueType::Graphics, .uses = cmdBufferUses}}};

  commandBufferManager->allocateCommandBuffers(commandBufferInfo);
  return pass;
}

auto R3Renderer::createGlobalShaderBindings() -> void {
  const auto defaultLayout = vk::DescriptorSetLayoutBinding{
      .binding = 0,
      .descriptorType = vk::DescriptorType::eCombinedImageSampler,
      .descriptorCount = 256,
      .stageFlags = vk::ShaderStageFlagBits::eFragment,
      .pImmutableSamplers = nullptr,
  };

  globalShaderBindings.defaultBindingLayout =
      layoutManager->createLayout(defaultLayout, "DefaultLayoutBinding");

  globalShaderBindings.defaultBinding =
      shaderBindingFactory->createShaderBinding(ShaderBindingType::Textures,
                                                globalShaderBindings.defaultBindingLayout);

  globalShaderBindings.defaultSampler = imageManager->registerDefaultSampler();

  const auto defaultSampler = imageManager->getSampler(globalShaderBindings.defaultSampler);

  for (const auto& frame : frameManager->getFrames()) {

    const auto bindingHandle = frame->getLogicalShaderBinding(globalShaderBindings.defaultBinding);
    auto& shaderBinding = shaderBindingFactory->getShaderBinding(bindingHandle);

    const auto imageHandle = frame->getLogicalImage(globalImages.forwardColorImage);
    const auto& colorImage = imageManager->getImage(imageHandle);

    const auto imguiImageHandle = frame->getLogicalImage(globalImages.imguiColorImage);
    const auto& imguiImage = imageManager->getImage(imguiImageHandle);

    std::vector<vk::DescriptorImageInfo> imageInfos = {
        vk::DescriptorImageInfo{
            .sampler = defaultSampler,
            .imageView = colorImage.getImageView(),
            .imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal,
        },
        vk::DescriptorImageInfo{
            .sampler = defaultSampler,
            .imageView = imguiImage.getImageView(),
            .imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal,
        }};

    shaderBinding.bindImageSamplers(0, imageInfos);
  }
}

}
