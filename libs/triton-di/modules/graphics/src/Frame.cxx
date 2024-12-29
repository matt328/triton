#include "Frame.hpp"

#include "cm/RenderData.hpp"
#include "gfx/IGraphicsDevice.hpp"
#include "mem/Image.hpp"
#include "renderer/IRenderer.hpp"
#include "mem/Buffer.hpp"
#include "sb/IShaderBindingFactory.hpp"
#include "sb/ShaderBinding.hpp"

namespace tr {
Frame::Frame(std::shared_ptr<IGraphicsDevice> newGraphicsDevice,
             std::shared_ptr<vk::raii::ImageView> newDepthImageView,
             const std::shared_ptr<IShaderBindingFactory>& shaderBindingFactory,
             std::string_view name)
    : frameName{name},
      commandBuffer{newGraphicsDevice->createCommandBuffer()},
      tracyContext{newGraphicsDevice->createTracyContext(name, *commandBuffer)},
      graphicsDevice{std::move(newGraphicsDevice)},
      depthImageView{std::move(newDepthImageView)} {

  constexpr auto fenceCreateInfo = vk::FenceCreateInfo{.flags = vk::FenceCreateFlagBits::eSignaled};
  inFlightFence =
      std::make_unique<vk::raii::Fence>(*graphicsDevice->getVulkanDevice(), fenceCreateInfo);
  imageAvailableSemaphore =
      std::make_unique<vk::raii::Semaphore>(*graphicsDevice->getVulkanDevice(),
                                            vk::SemaphoreCreateInfo{});

  renderFinishedSemaphore =
      std::make_unique<vk::raii::Semaphore>(*graphicsDevice->getVulkanDevice(),
                                            vk::SemaphoreCreateInfo{});

  objectDataBuffer = graphicsDevice->createStorageBuffer(sizeof(ObjectData) * MAX_OBJECTS,
                                                         frameName + " Object Data");
  objectDataBuffer->mapBuffer();

  cameraDataBuffer =
      graphicsDevice->createUniformBuffer(sizeof(CameraData), frameName + " Camera Data");
  cameraDataBuffer->mapBuffer();

  animationDataBuffer = graphicsDevice->createStorageBuffer(sizeof(GpuAnimationData) * MAX_OBJECTS,
                                                            frameName + " Animation Data");
  animationDataBuffer->mapBuffer();

  std::tie(drawImage, drawImageView) = graphicsDevice->createDrawImage("Draw Image");

  perFrameShaderBinding = shaderBindingFactory->createShaderBinding(ShaderBindingHandle::PerFrame);
  perFrameShaderBinding->bindBuffer(0, *cameraDataBuffer, sizeof(::CameraData));

  objectDataShaderBinding =
      shaderBindingFactory->createShaderBinding(ShaderBindingHandle::ObjectData);
  objectDataShaderBinding->bindBuffer(0, *objectDataBuffer, sizeof(::ObjectData) * MAX_OBJECTS);

  textureShaderBinding = shaderBindingFactory->createShaderBinding(ShaderBindingHandle::Bindless);

  animationDataShaderBinding =
      shaderBindingFactory->createShaderBinding(ShaderBindingHandle::AnimationData);
  animationDataShaderBinding->bindBuffer(0,
                                         *animationDataBuffer,
                                         sizeof(GpuAnimationData) * MAX_OBJECTS);
  createSwapchainResources();
}

Frame::~Frame() {
  Log.trace("Destroying {0}", frameName);
}

void Frame::registerStorageBuffer([[maybe_unused]] const std::string& name,
                                  [[maybe_unused]] size_t size) {
}

void Frame::destroySwapchainResources() {
  commandBuffer.reset();
}

void Frame::createSwapchainResources() {
  commandBuffer = graphicsDevice->createCommandBuffer();
}

void Frame::awaitInFlightFence() {
  if (const auto res =
          graphicsDevice->getVulkanDevice()->waitForFences(**inFlightFence, VK_TRUE, UINT64_MAX);
      res != vk::Result::eSuccess) {
    throw std::runtime_error("Error waiting for fences");
  }
}

auto Frame::acquireSwapchainImage() -> AcquireResult {
  ZoneNamedN(acquire, "Acquire Swapchain Image", true);
  const auto result = graphicsDevice->acquireNextSwapchainImage(*imageAvailableSemaphore);
  if (std::holds_alternative<uint32_t>(result)) {
    swapchainImageIndex = std::get<uint32_t>(result);
    return AcquireResult::Success;
  }
  return std::get<AcquireResult>(result);
}

void Frame::resetInFlightFence() {
  graphicsDevice->getVulkanDevice()->resetFences(**inFlightFence);
}

void Frame::applyRenderData(constRenderData& renderData) {
  staticMeshDataList.clear();
  lineDataList.clear();
  terrainDataList.clear();
  skinnedModelList.clear();
  {
    ZoneNamedN(zone, "Copying RenderData", true);
    updateObjectDataBuffer(renderData.objectData.data(),
                           sizeof(::ObjectData) * renderData.objectData.size());

    updatePerFrameDataBuffer(&renderData.cameraData, sizeof(::CameraData));

    // TODO(Matt): Rename this AnimationData class
    updateAnimationDataBuffer(renderData.animationData.data(),
                              sizeof(GpuAnimationData) * renderData.animationData.size());

    auto meshData = renderData.staticMeshData;

    auto it = std::partition(meshData.begin(), meshData.end(), [](auto& meshData) {
      return meshData.topology == Topology::Triangles;
    });
    staticMeshDataList.assign(meshData.begin(), it);
    lineDataList.assign(it, meshData.end());

    terrainDataList.reserve(renderData.terrainMeshData.size());
    std::ranges::copy(renderData.terrainMeshData, std::back_inserter(terrainDataList));

    skinnedModelList.reserve(renderData.skinnedMeshData.size());
    std::ranges::copy(renderData.skinnedMeshData, std::back_inserter(skinnedModelList));

    pushConstants = renderData.pushConstants;
  }
}

void Frame::applyTextures(const std::vector<vk::DescriptorImageInfo>& imageInfo) {
  if (!imageInfo.empty()) {
    textureShaderBinding->bindImageSamplers(3, imageInfo);
  }
}

void Frame::render(const std::shared_ptr<rd::IRenderer>& renderer,
                   const std::tuple<vk::Viewport, vk::Rect2D>& vpScissor) {
  commandBuffer->begin(
      vk::CommandBufferBeginInfo{.flags = vk::CommandBufferUsageFlagBits::eSimultaneousUse});
  {
    TracyVkZone(tracyContext.get(), **commandBuffer, "Prep Frame");
    prepareFrame();

    {
      ZoneNamedN(zone2, "Apply Shader Bindings", true);
      renderer->bindPipeline(commandBuffer);
      renderer->applyShaderBinding(textureShaderBinding, 0, commandBuffer);
      renderer->applyShaderBinding(objectDataShaderBinding, 1, commandBuffer);
      renderer->applyShaderBinding(perFrameShaderBinding, 2, commandBuffer);
    }
    renderer->render(commandBuffer, staticMeshDataList, vpScissor);
  }
}
void Frame::prepareFrame() {
  Frame::transitionImage(*commandBuffer,
                         drawImage->getImage(),
                         vk::ImageLayout::eUndefined,
                         vk::ImageLayout::eColorAttachmentOptimal);

  const auto colorAttachmentInfo = vk::RenderingAttachmentInfo{
      .imageView = **drawImageView,
      .imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
      .loadOp = vk::AttachmentLoadOp::eClear,
      .storeOp = vk::AttachmentStoreOp::eStore,
      .clearValue = vk::ClearValue{.color = vk::ClearColorValue{std::array<float, 4>(
                                       {{0.39f, 0.58f, 0.93f, 1.f}})}},
  };

  const auto depthAttachmentInfo = vk::RenderingAttachmentInfo{
      .imageView = **depthImageView,
      .imageLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal,
      .loadOp = vk::AttachmentLoadOp::eClear,
      .storeOp = vk::AttachmentStoreOp::eStore,
      .clearValue =
          vk::ClearValue{.depthStencil = vk::ClearDepthStencilValue{.depth = 1.f, .stencil = 0}},
  };

  const auto renderingInfo = vk::RenderingInfo{
      .renderArea = vk::Rect2D{.offset = {0, 0}, .extent = IGraphicsDevice::DrawImageExtent2D},
      .layerCount = 1,
      .colorAttachmentCount = 1,
      .pColorAttachments = &colorAttachmentInfo,
      .pDepthAttachment = &depthAttachmentInfo};

  commandBuffer->beginRendering(renderingInfo);
}

auto Frame::renderGuiSystem(const std::shared_ptr<IGuiSystem>& guiSystem) -> void {
  guiSystem->render(commandBuffer,
                    graphicsDevice->getSwapchainImageView(swapchainImageIndex),
                    graphicsDevice->getSwapchainExtent());
}

auto Frame::end3d() -> void {
  commandBuffer->endRendering();

  const auto& swapchainImage = graphicsDevice->getSwapchainImage(swapchainImageIndex);
  const auto& swapchainExtent = graphicsDevice->getSwapchainExtent();

  graphicsDevice->transitionImage(*commandBuffer,
                                  drawImage->getImage(),
                                  vk::ImageLayout::eColorAttachmentOptimal,
                                  vk::ImageLayout::eTransferSrcOptimal);
  graphicsDevice->transitionImage(*commandBuffer,
                                  swapchainImage,
                                  vk::ImageLayout::eUndefined,
                                  vk::ImageLayout::eTransferDstOptimal);

  graphicsDevice->copyImageToImage(*commandBuffer,
                                   drawImage->getImage(),
                                   swapchainImage,
                                   IGraphicsDevice::DrawImageExtent2D,
                                   swapchainExtent);

  graphicsDevice->transitionImage(*commandBuffer,
                                  swapchainImage,
                                  vk::ImageLayout::eTransferDstOptimal,
                                  vk::ImageLayout::eColorAttachmentOptimal);

  transitionImage(*commandBuffer,
                  swapchainImage,
                  vk::ImageLayout::eColorAttachmentOptimal,
                  vk::ImageLayout::ePresentSrcKHR);

  TracyVkCollect(tracyContext, **commandBuffer);
}

auto Frame::present() -> bool {

  commandBuffer->end();

  bool recreateSwapchain{};
  constexpr auto waitStages =
      std::array<vk::PipelineStageFlags, 1>{vk::PipelineStageFlagBits::eColorAttachmentOutput};

  const auto renderFinishedSemaphores = std::array<vk::Semaphore, 1>{*renderFinishedSemaphore};

  const auto submitInfo = vk::SubmitInfo{.waitSemaphoreCount = 1,
                                         .pWaitSemaphores = &**imageAvailableSemaphore,
                                         .pWaitDstStageMask = waitStages.data(),
                                         .commandBufferCount = 1,
                                         .pCommandBuffers = &**commandBuffer,
                                         .signalSemaphoreCount = 1,
                                         .pSignalSemaphores = renderFinishedSemaphores.data()};

  graphicsDevice->submit(submitInfo, inFlightFence);

  try {
    if (const auto result2 = graphicsDevice->present(renderFinishedSemaphore, swapchainImageIndex);
        result2 == vk::Result::eSuboptimalKHR) {
      recreateSwapchain = true;
    }
  } catch (const std::exception& ex) {
    Log.info("Swapchain needs recreated: {0}", ex.what());
    recreateSwapchain = true;
  }
  return recreateSwapchain;
}

void Frame::updateObjectDataBuffer(const ObjectData* data, size_t size) const {
  objectDataBuffer->updateMappedBufferValue(data, size);
}
void Frame::updatePerFrameDataBuffer(const CameraData* data, size_t size) const {
  cameraDataBuffer->updateMappedBufferValue(data, size);
}
void Frame::updateAnimationDataBuffer(const GpuAnimationData* data, size_t size) const {
  animationDataBuffer->updateMappedBufferValue(data, size);
}

void Frame::transitionImage(const vk::raii::CommandBuffer& cmd,
                            const vk::Image& image,
                            const vk::ImageLayout currentLayout,
                            const vk::ImageLayout newLayout) {
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
}
