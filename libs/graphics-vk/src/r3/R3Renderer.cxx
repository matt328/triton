#include "R3Renderer.hpp"
#include "api/fx/IEventBus.hpp"
#include "api/gw/RenderableResources.hpp"
#include "gfx/IFrameGraph.hpp"
#include "gfx/IFrameManager.hpp"
#include "gfx/QueueTypes.hpp"
#include "mem/GeometryBuffer.hpp"
#include "r3/render-pass/ComputePass.hpp"
#include "r3/render-pass/RenderPassFactory.hpp"
#include "task/Frame.hpp"
#include "render-pass/GraphicsPassCreateInfo.hpp"
#include "gfx/PassGraphInfo.hpp"
#include "vk/ComputePushConstants.hpp"
#include "mem/buffer-registry/BufferRequest.hpp"

namespace tr {

R3Renderer::R3Renderer(RenderContextConfig newRenderConfig,
                       std::shared_ptr<IFrameManager> newFrameManager,
                       std::shared_ptr<queue::Graphics> newGraphicsQueue,
                       std::shared_ptr<IEventBus> newEventBus,
                       std::shared_ptr<Swapchain> newSwapchain,
                       std::shared_ptr<IFrameGraph> newFrameGraph,
                       std::shared_ptr<RenderPassFactory> newRenderPassFactory,
                       std::shared_ptr<CommandBufferManager> newCommandBufferManager)
    : rendererConfig{newRenderConfig},
      frameManager{std::move(newFrameManager)},
      graphicsQueue{std::move(newGraphicsQueue)},
      eventBus{std::move(newEventBus)},
      swapchain{std::move(newSwapchain)},
      frameGraph{std::move(newFrameGraph)},
      renderPassFactory{std::move(newRenderPassFactory)},
      commandBufferManager{std::move(newCommandBufferManager)} {

  /*
    TODO(matt): createComputeCullingPass()
    - register a fullscreen quad geometry specifically for use by the composition pass to write onto
    the swapchain image.
    - This will exercise all the geometry buffer and basically the entire pipeline.
    - Start with culling pass. Make sure it creates the correct DIIC, count, and uses the DIIC
    metadata buffer
  */
  createGlobalBuffers();
  createComputeCullingPass();
  createForwardRenderPass();
  createCompositionRenderPass();
}

/*
  TODO(matt): Simplify and streamline buffer creation.
  For now, everything goes through the framemanager in case it needs to be a per frame buffer.
  Reconsider moving it around so everything goes through the BufferRegistry, and that has the
  frameManager injected into it so that it can transparently register per frame buffers and return
  logical handles. That way the buffer registry can also track if the handle belongs to a per-frame
  buffer. After creating a per-frame buffer, a LogicalHandle will be returned. LogicalHandles must
  only be passed to frames to get the Handle, which will then be passed to the bufferRegistry. If a
  non-per frame buffer is registered and then requested, this will be purely with the
  bufferRegistry.
*/

auto R3Renderer::createGlobalBuffers() -> void {
  globalBuffers.drawCommands = frameManager->registerBufferRequest(BufferRequest{});
  globalBuffers.drawCounts = frameManager->registerBufferRequest(BufferRequest{});
  globalBuffers.drawMetadata = frameManager->registerBufferRequest(BufferRequest{});
  globalBuffers.geometry = frameManager->registerBufferRequest(BufferRequest{});
}

auto R3Renderer::registerRenderable([[maybe_unused]] const RenderableData& data)
    -> RenderableResources {
  // This won't actually need to be here. The Renderer will already 'expect' the things that can be
  // rendered
  return RenderableResources{};
}

void R3Renderer::update() {
}

void R3Renderer::renderNextFrame() {
  const auto result = frameManager->acquireFrame();

  if (std::holds_alternative<ImageAcquireResult>(result)) {
    if (const auto acquireResult = std::get<ImageAcquireResult>(result);
        acquireResult == ImageAcquireResult::Error) {
      Log.warn("Failed to acquire swapchain image");
      return;
    }
  }
  if (std::holds_alternative<Frame*>(result)) {
    const auto* frame = std::get<Frame*>(result);

    const auto& results = frameGraph->execute(frame);

    endFrame(frame, results);
  }
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
    eventBus->emit(FrameEndEvent{fence});
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

auto R3Renderer::getGeometryBuffer() -> GeometryBuffer& {
  return *geometryBuffer;
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
                                         .shaderFile = SHADER_ROOT / "compute.comp.spv",
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

  const auto cullingPassGraphInfo = PassGraphInfo{};

  frameGraph->addPass(std::move(cullingPass), cullingPassGraphInfo);
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

  const auto forwardPassCreateInfo =
      GraphicsPassCreateInfo{.id = "forward",
                             .pipelineLayoutInfo = PipelineLayoutInfo{},
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

  const auto forwardPassGraphInfo = PassGraphInfo{};

  frameGraph->addPass(std::move(forwardPass), forwardPassGraphInfo);
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

  const auto compositionPassGraphInfo = PassGraphInfo{};

  frameGraph->addPass(std::move(compositionPass), compositionPassGraphInfo);
}

}
