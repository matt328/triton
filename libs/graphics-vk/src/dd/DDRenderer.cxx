#include "dd/DDRenderer.hpp"
#include "api/fx/IEventBus.hpp"
#include "dd/DrawContextFactory.hpp"
#include "dd/RenderConfigRegistry.hpp"
#include "dd/render-pass/RenderPassCreateInfo.hpp"
#include "dd/render-pass/RenderPassFactory.hpp"
#include "gfx/IFrameManager.hpp"
#include "img/ImageRequest.hpp"
#include "vk/core/Swapchain.hpp"
#include "task/Frame.hpp"

namespace tr {

DDRenderer::DDRenderer(RenderContextConfig newConfig,
                       std::shared_ptr<RenderConfigRegistry> newRenderConfigRegistry,
                       std::shared_ptr<DrawContextFactory> newDrawContextFactory,
                       std::shared_ptr<IFrameManager> newFrameManager,
                       std::shared_ptr<RenderPassFactory> newRenderPassFactory,
                       std::shared_ptr<CommandBufferManager> newCommandBufferManager,
                       std::shared_ptr<VkResourceManager> newResourceManager,
                       std::shared_ptr<Swapchain> newSwapchain,
                       std::shared_ptr<queue::Graphics> newGraphicsQueue,
                       std::shared_ptr<IEventBus> newEventBus)
    : rendererConfig{newConfig},
      renderConfigRegistry{std::move(newRenderConfigRegistry)},
      drawContextFactory{std::move(newDrawContextFactory)},
      frameManager{std::move(newFrameManager)},
      renderPassFactory{std::move(newRenderPassFactory)},
      commandBufferManager{std::move(newCommandBufferManager)},
      resourceManager{std::move(newResourceManager)},
      swapchain{std::move(newSwapchain)},
      graphicsQueue{std::move(newGraphicsQueue)},
      eventBus{std::move(newEventBus)} {

  Log.trace("Constructing Data Driven Renderer");

  const auto extent =
      vk::Extent2D{.width = rendererConfig.initialWidth, .height = rendererConfig.initialHeight};

  const auto sceneColor = ImageRequest{
      .logicalName = "scene.color",
      .format = vk::Format::eR16G16B16A16Sfloat,
      .extent = extent,
      .usageFlags = vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst |
                    vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eColorAttachment,
      .aspectFlags = vk::ImageAspectFlagBits::eColor,
      .debugName = "SceneColor"};

  const auto sceneDepth =
      ImageRequest{.logicalName = "scene.depth",
                   .format = swapchain->getDepthFormat(),
                   .extent = extent,
                   .usageFlags = vk::ImageUsageFlagBits::eDepthStencilAttachment,
                   .aspectFlags = vk::ImageAspectFlagBits::eDepth,
                   .debugName = "SceneDepth"};

  const auto uiColor = ImageRequest{
      .logicalName = "ui.color",
      .format = vk::Format::eR16G16B16A16Sfloat,
      .extent = extent,
      .usageFlags = vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst |
                    vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eColorAttachment,
      .aspectFlags = vk::ImageAspectFlagBits::eColor,
      .debugName = "UIColor"};

  auto sceneColorHandle = frameManager->registerImageRequest(sceneColor);
  auto sceneDepthHandle = frameManager->registerImageRequest(sceneDepth);
  auto uiColorHandle = frameManager->registerImageRequest(uiColor);

  auto forwardPassCreateInfo =
      RenderPassCreateInfo{.colorAttachments = {AttachmentConfig{.logicalImage = sceneColorHandle}},
                           .depthAttachment = AttachmentConfig{.logicalImage = sceneDepthHandle}};
  renderPasses.emplace_back(renderPassFactory->createRenderPass(forwardPassCreateInfo));

  auto uiPassCreateInfo =
      RenderPassCreateInfo{.colorAttachments = {AttachmentConfig{.logicalImage = uiColorHandle}}};
  renderPasses.emplace_back(renderPassFactory->createRenderPass(uiPassCreateInfo));
}

auto DDRenderer::update() -> void {
  // Not really used. Can probably get rid of it
}

auto DDRenderer::registerRenderable(const RenderableData& data) -> RenderableResources {
  // Eventually move viewport and scissor so they're defined in RenderableData

  auto objectDataType = ObjectDataType::Base;
  if (data.materialData.imageData) {
    objectDataType = ObjectDataType::BaseMaterial;
  } else if (data.animationData) {
    objectDataType = ObjectDataType::BaseMaterialAnimated;
  }

  const auto renderConfig =
      RenderConfig{.vertexFormat = data.geometryData.getVertexList().format,
                   .topology = data.geometryData.getVertexList().topology,
                   .shadingMode = data.materialData.shadingMode,
                   .viewport = Viewport{.width = swapchain->getImageExtent().width,
                                        .height = swapchain->getImageExtent().height},
                   .scissor = Scissor{.width = swapchain->getImageExtent().width,
                                      .height = swapchain->getImageExtent().height},
                   .objectDataType = objectDataType};

  const auto renderConfigHandle = renderConfigRegistry->registerOrGet(renderConfig);

  auto* const drawContext = drawContextFactory->getOrCreateDrawContext(renderConfigHandle);

  for (const auto& renderPass : renderPasses) {
    if (renderPass->accepts(renderConfig)) {
      renderPass->addDrawContext(renderConfigHandle, drawContext);
    }
  }

  return drawContext->registerRenderable(data);
}

auto DDRenderer::renderNextFrame() -> void {
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

    preRender(frame);

    auto& commandBuffer =
        commandBufferManager->getCommandBuffer(frame->getMainCommandBufferHandle());

    commandBuffer.begin(
        vk::CommandBufferBeginInfo{.flags = vk::CommandBufferUsageFlagBits::eSimultaneousUse});

    for (auto& pass : renderPasses) {
      pass->execute(frame, commandBuffer);
    }

    commandBuffer.end();

    combineImages(frame);

    endFrame(frame);
  }
}

auto DDRenderer::preRender(const Frame* frame) -> void {
  // TODO(matt): RenderPasses will be responsible for ensuring images are transitioned into the
  // expected layout ZoneNamedN(var, "Record Command Buffers", true);
  // {
  //   ZoneNamedN(var, "Start", true);
  //   const auto& startCmd =
  //       commandBufferManager->getCommandBuffer(frame->getStartCommandBufferHandle());
  //   startCmd.begin(
  //       vk::CommandBufferBeginInfo{.flags = vk::CommandBufferUsageFlagBits::eSimultaneousUse});

  //   utils::transitionImage(startCmd,
  //                          resourceManager->getImage(frame->getDrawImageHandle()),
  //                          vk::ImageLayout::eUndefined,
  //                          vk::ImageLayout::eColorAttachmentOptimal);

  //   startCmd.end();
  // }
}

auto DDRenderer::waitIdle() -> void {
}

auto DDRenderer::setRenderData(const RenderData& newRenderData) -> void {
  renderData = newRenderData;
}

// TODO(matt): Replace this blit function with a renderpass? that combines all the images into a
// final image
auto DDRenderer::combineImages([[maybe_unused]] const Frame* frame) -> void {
  // ZoneNamedN(var, "End", true);
  // auto& endCmd = commandBufferManager->getCommandBuffer(frame->getEndCommandBufferHandle());
  // endCmd.begin(
  //     vk::CommandBufferBeginInfo{.flags = vk::CommandBufferUsageFlagBits::eSimultaneousUse});

  // utils::transitionImage(endCmd,
  //                        resourceManager->getImage(frame->getDrawImageHandle()),
  //                        vk::ImageLayout::eColorAttachmentOptimal,
  //                        vk::ImageLayout::eTransferSrcOptimal);

  // utils::transitionImage(endCmd,
  //                        swapchain->getSwapchainImage(frame->getSwapchainImageIndex()),
  //                        vk::ImageLayout::eUndefined,
  //                        vk::ImageLayout::eTransferDstOptimal);

  // utils::copyImageToImage(endCmd,
  //                         resourceManager->getImage(frame->getDrawImageHandle()),
  //                         swapchain->getSwapchainImage(frame->getSwapchainImageIndex()),
  //                         resourceManager->getImageExtent(frame->getDrawImageHandle()),
  //                         swapchain->getImageExtent());

  // // guiSystem->render(endCmd,
  // //                   swapchain->getSwapchainImageView(frame->getSwapchainImageIndex()),
  // //                   swapchain->getImageExtent());

  // utils::transitionImage(endCmd,
  //                        swapchain->getSwapchainImage(frame->getSwapchainImageIndex()),
  //                        vk::ImageLayout::eTransferDstOptimal,
  //                        vk::ImageLayout::ePresentSrcKHR);

  // endCmd.end();
}

auto DDRenderer::endFrame(const Frame* frame) -> void {
  buffers.clear();
  buffers.push_back(*commandBufferManager->getCommandBuffer(frame->getStartCommandBufferHandle()));
  buffers.push_back(*commandBufferManager->getCommandBuffer(frame->getMainCommandBufferHandle()));
  buffers.push_back(*commandBufferManager->getCommandBuffer(frame->getEndCommandBufferHandle()));

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
}
