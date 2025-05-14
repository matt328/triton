#include "R3Renderer.hpp"
#include "api/gw/RenderableResources.hpp"
#include "buffers/BufferCreateInfo.hpp"
#include "buffers/BufferSystem.hpp"
#include "gfx/IFrameGraph.hpp"
#include "gfx/IFrameManager.hpp"
#include "gfx/QueueTypes.hpp"
#include "r3/draw-context/ContextFactory.hpp"
#include "r3/draw-context/DispatchContext.hpp"
#include "r3/render-pass/ComputePass.hpp"
#include "r3/render-pass/RenderPassFactory.hpp"
#include "task/Frame.hpp"
#include "render-pass/GraphicsPassCreateInfo.hpp"
#include "gfx/PassGraphInfo.hpp"
#include "vk/ComputePushConstants.hpp"

namespace tr {

constexpr std::string CullingPassId = "pass.culling";
constexpr std::string ForwardId = "pass.forward";

constexpr std::string CubeDrawContext = "context.cube";
constexpr std::string CullingDispatchContext = "context.culling";

const std::unordered_map<std::string, std::vector<std::string>> GraphicsMap = {
    {CubeDrawContext, {ForwardId}}};

const std::unordered_map<std::string, std::vector<std::string>> ComputeMap = {
    {CullingDispatchContext, {CullingPassId}}};

R3Renderer::R3Renderer(RenderContextConfig newRenderConfig,
                       std::shared_ptr<IFrameManager> newFrameManager,
                       std::shared_ptr<queue::Graphics> newGraphicsQueue,
                       std::shared_ptr<Swapchain> newSwapchain,
                       std::shared_ptr<IFrameGraph> newFrameGraph,
                       std::shared_ptr<RenderPassFactory> newRenderPassFactory,
                       std::shared_ptr<CommandBufferManager> newCommandBufferManager,
                       std::shared_ptr<BufferSystem> newBufferSystem,
                       std::shared_ptr<ContextFactory> newDrawContextFactory,
                       std::shared_ptr<IStateBuffer> newStateBuffer)
    : rendererConfig{newRenderConfig},
      frameManager{std::move(newFrameManager)},
      graphicsQueue{std::move(newGraphicsQueue)},
      swapchain{std::move(newSwapchain)},
      frameGraph{std::move(newFrameGraph)},
      renderPassFactory{std::move(newRenderPassFactory)},
      commandBufferManager{std::move(newCommandBufferManager)},
      bufferSystem{std::move(newBufferSystem)},
      drawContextFactory{std::move(newDrawContextFactory)},
      stateBuffer{std::move(newStateBuffer)} {

  drawContextFactory->createDrawContext(
      CubeDrawContext,
      DrawContextConfig{.logicalBuffers = {},
                        .indirectBuffer = globalBuffers.drawCommands,
                        .countBuffer = globalBuffers.drawCounts,
                        .indirectMetadata = IndirectMetadata{}});
  drawContextFactory->createDispatchContext(CullingDispatchContext,
                                            DispatchContextConfig{.logicalBuffers = {}});
  /*
    TODO(matt): createComputeCullingPass()
    - register a fullscreen quad geometry specifically for use by the composition pass to write onto
    the swapchain image.
    - This will exercise all the geometry buffer and basically the entire pipeline.
    - Start with culling pass. Make sure it creates the correct DIIC, count, and uses the DIIC
    metadata buffer

    Culling Shader:
    - todo tomorrow
  */
  createGlobalBuffers();
  createComputeCullingPass();
  createForwardRenderPass();
  // createCompositionRenderPass();

  for (const auto& [contextId, passIds] : GraphicsMap) {
    for (const auto& passId : passIds) {
      frameGraph->getGraphicsPass(passId)->registerDrawContext(
          drawContextFactory->getDrawContextHandle(contextId));
    }
  }

  for (const auto& [contextId, passIds] : ComputeMap) {
    for (const auto& passId : passIds) {
      frameGraph->getComputePass(passId)->registerDispatchContext(
          drawContextFactory->getDispatchContextHandle(contextId));
    }
  }
}

auto R3Renderer::createGlobalBuffers() -> void {
  globalBuffers.drawCommands = bufferSystem->registerPerFrameBuffer(
      BufferCreateInfo{.bufferType = BufferType::IndirectCommand, .initialSize = 10240});

  globalBuffers.drawCounts = bufferSystem->registerPerFrameBuffer(
      BufferCreateInfo{.bufferType = BufferType::Device, .initialSize = 64});

  // Specifies how the DIIC and DrawCounts buffers are sliced, providing maximum space as if all
  // renderables would be rendered this frame. Determines offset values for DIIC and DrawCounts
  // buffers during drawIndexedIndirect command redording.
  globalBuffers.drawMetadata = bufferSystem->registerPerFrameBuffer(BufferCreateInfo{
      .bufferType = BufferType::HostTransient,
  });

  globalBuffers.objectData = bufferSystem->registerPerFrameBuffer(
      BufferCreateInfo{.bufferType = BufferType::HostTransient});

  globalBuffers.geometryEntry =
      bufferSystem->registerBuffer(BufferCreateInfo{.bufferType = BufferType::DeviceArena,
                                                    .itemStride = sizeof(GpuGeometryRegionData)});

  globalBuffers.geometryPositions = bufferSystem->registerBuffer(
      BufferCreateInfo{.bufferType = BufferType::DeviceArena, .itemStride = sizeof(glm::vec3)});

  globalBuffers.geometryColors = bufferSystem->registerBuffer(
      BufferCreateInfo{.bufferType = BufferType::DeviceArena, .itemStride = sizeof(glm::vec4)});
}

auto R3Renderer::registerRenderable([[maybe_unused]] const RenderableData& data)
    -> RenderableResources {
  // This won't actually need to be here. The Renderer will already 'expect' the things that can be
  // rendered
  return RenderableResources{};
}

void R3Renderer::update() {
  /*
    TODO(matt): Sketch out how the client can write state into a ring buffer on a fixed interval,
    and how the renderer can pick up 2 states and an interpolation value from this ring buffer on
    it's own interval.

  */
}

void R3Renderer::setStates(SimState previous, SimState next, float alpha) {
}

void R3Renderer::renderNextFrame() {
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
  // Process sync point here
  SimState current{1};
  SimState prev{1};
  float alpha{0};
  Timestamp currentTime = std::chrono::steady_clock::now();
  stateBuffer->getInterpolatedStates(current, prev, alpha, currentTime);
  const auto& results = frameGraph->execute(frame);
  endFrame(frame, results);
}

auto R3Renderer::endFrame(const Frame* frame, const FrameGraphResult& results) -> void {
  buffers.clear();

  constexpr auto waitStages =
      std::array<vk::PipelineStageFlags, 1>{vk::PipelineStageFlagBits::eColorAttachmentOutput};

  const auto submitInfo = vk::SubmitInfo{
      .waitSemaphoreCount = 1,
      .pWaitSemaphores = &*frame->getImageAvailableSemaphore(),
      .pWaitDstStageMask = waitStages.data(),
      .commandBufferCount = static_cast<uint32_t>(buffers.size()),
      .pCommandBuffers = buffers.data(),
      .signalSemaphoreCount = 1,
      .pSignalSemaphores = &*frame->getRenderFinishedSemaphore(),
  };

  std::string msg = fmt::format("Submitting Queue for frame {}", frame->getIndex());
  TracyMessage(msg.data(), msg.size());

  try {
    const auto fence = *frame->getInFlightFence();
    graphicsQueue->getQueue().submit(submitInfo, fence);
  } catch (const std::exception& ex) {
    Log.error("Failed to submit command buffer submission {}", ex.what());
  }

  try {
    const auto swapchainImageIndex = frame->getSwapchainImageIndex();
    const auto chain = swapchain->getSwapchain();

    const auto presentInfo =
        vk::PresentInfoKHR{.waitSemaphoreCount = 1,
                           .pWaitSemaphores = &*frame->getRenderFinishedSemaphore(),
                           .swapchainCount = 1,
                           .pSwapchains = &chain,
                           .pImageIndices = &swapchainImageIndex};

    std::string msg = fmt::format("Presenting frame {}", frame->getIndex());
    TracyMessage(msg.data(), msg.size());
    if (const auto result2 = graphicsQueue->getQueue().presentKHR(presentInfo);
        result2 == vk::Result::eSuboptimalKHR || result2 == vk::Result::eErrorOutOfDateKHR) {
      Log.trace("Swapchain Needs Resized");
    }
  } catch (const std::exception& ex) { Log.trace("Swapchain needs recreated: {0}", ex.what()); }
}

void R3Renderer::waitIdle() {
}

void R3Renderer::setRenderData([[maybe_unused]] const RenderData& renderData) {
}

auto R3Renderer::createComputeCullingPass() -> void {

  const auto pipelineLayoutInfo =
      PipelineLayoutInfo{.pushConstantInfoList = {PushConstantInfo{
                             .stageFlags = vk::ShaderStageFlagBits::eCompute,
                             .offset = 0,
                             .size = sizeof(ComputePushConstants),
                         }}};

  const auto cullingPassInfo = ComputePassCreateInfo{
      .id = "culling",
      .pipelineLayoutInfo = pipelineLayoutInfo,
      .shaderStageInfo = ShaderStageInfo{.stage = vk::ShaderStageFlagBits::eCompute,
                                         .shaderFile = SHADER_ROOT / "compute2.comp.spv",
                                         .entryPoint = "main"}};
  auto cullingPass = renderPassFactory->createComputePass(cullingPassInfo);

  auto uses = std::vector<CommandBufferUse>{};

  for (const auto& frame : frameManager->getFrames()) {
    uses.emplace_back(CommandBufferUse{.threadId = std::this_thread::get_id(),
                                       .frameId = frame->getIndex(),
                                       .passId = cullingPass->getId()});
  }

  const auto commandBufferInfo = CommandBufferInfo{
      .queueConfigs = {QueueConfig{.queueType = QueueType::Compute, .uses = uses}}};
  commandBufferManager->allocateCommandBuffers(commandBufferInfo);

  frameGraph->addPass(std::move(cullingPass), PassGraphInfo{.id = CullingPassId});
}

auto R3Renderer::createForwardRenderPass() -> void {
  auto colorAttachmentInfo = AttachmentCreateInfo{
      .format = vk::Format::eR16G16B16A16Sfloat,
      .clearValue = vk::ClearValue{
          .color = vk::ClearColorValue{std::array<float, 4>{0.392f, 0.584f, 0.929f, 1.0f}}}};

  const auto vertexStage = ShaderStageInfo{
      .stage = vk::ShaderStageFlagBits::eVertex,
      .shaderFile = SHADER_ROOT / "static.vert.spv",
      .entryPoint = "main",
  };

  const auto fragmentStage = ShaderStageInfo{
      .stage = vk::ShaderStageFlagBits::eFragment,
      .shaderFile = SHADER_ROOT / "static.frag.spv",
      .entryPoint = "main",
  };

  const auto forwardPassCreateInfo = GraphicsPassCreateInfo{
      .id = "forward",
      .pipelineLayoutInfo = PipelineLayoutInfo{.pushConstantInfoList = {PushConstantInfo{
                                                   .stageFlags = vk::ShaderStageFlagBits::eVertex,
                                                   .offset = 0,
                                                   .size = 36}}}, // TODO(matt): Fix this
      .colorAttachmentInfos = {colorAttachmentInfo},
      .shaderStageInfo = {vertexStage, fragmentStage},
      .extent = vk::Extent2D{.width = rendererConfig.initialWidth,
                             .height = rendererConfig.initialHeight}};

  auto forwardPass = renderPassFactory->createGraphicsPass(forwardPassCreateInfo);

  std::vector<CommandBufferUse> uses{};

  for (const auto& frame : frameManager->getFrames()) {
    uses.emplace_back(CommandBufferUse{.threadId = std::this_thread::get_id(),
                                       .frameId = frame->getIndex(),
                                       .passId = forwardPass->getId()});
  }

  const auto commandBufferInfo = CommandBufferInfo{
      .queueConfigs = {QueueConfig{.queueType = QueueType::Graphics, .uses = uses}}};

  commandBufferManager->allocateCommandBuffers(commandBufferInfo);

  frameGraph->addPass(std::move(forwardPass), PassGraphInfo{.id = ForwardId});
}

auto R3Renderer::createCompositionRenderPass() -> void {
  auto colorAttachmentInfo = AttachmentCreateInfo{
      .format = vk::Format::eR16G16B16A16Sfloat,
      .clearValue = vk::ClearValue{
          .color = vk::ClearColorValue{std::array<float, 4>{0.392f, 0.584f, 0.929f, 1.0f}}}};

  const auto vertexStage = ShaderStageInfo{
      .stage = vk::ShaderStageFlagBits::eVertex,
      .shaderFile = SHADER_ROOT / "composition.vert.spv",
      .entryPoint = "main",
  };

  const auto fragmentStage = ShaderStageInfo{
      .stage = vk::ShaderStageFlagBits::eFragment,
      .shaderFile = SHADER_ROOT / "composition.frag.spv",
      .entryPoint = "main",
  };

  const auto compositionPassCreateInfo =
      GraphicsPassCreateInfo{.id = "composition",
                             .pipelineLayoutInfo = PipelineLayoutInfo{},
                             .colorAttachmentInfos = {colorAttachmentInfo},
                             .shaderStageInfo = {vertexStage, fragmentStage},
                             .extent = vk::Extent2D{.width = rendererConfig.initialWidth,
                                                    .height = rendererConfig.initialHeight}};

  auto compositionPass = renderPassFactory->createGraphicsPass(compositionPassCreateInfo);

  std::vector<CommandBufferUse> uses{};

  for (const auto& frame : frameManager->getFrames()) {
    uses.emplace_back(CommandBufferUse{.threadId = std::this_thread::get_id(),
                                       .frameId = frame->getIndex(),
                                       .passId = compositionPass->getId()});
  }

  const auto commandBufferInfo = CommandBufferInfo{
      .queueConfigs = {QueueConfig{.queueType = QueueType::Graphics, .uses = uses}}};

  commandBufferManager->allocateCommandBuffers(commandBufferInfo);

  frameGraph->addPass(std::move(compositionPass), PassGraphInfo{.id = CullingPassId});
}

}
