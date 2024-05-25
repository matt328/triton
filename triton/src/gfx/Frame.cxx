#include "Frame.hpp"
#include "gfx/ObjectData.hpp"
#include "GraphicsDevice.hpp"
#include "gfx/sb/LayoutFactory.hpp"
#include "gfx/helpers/Rendering.hpp"
#include "gfx/mem/Buffer.hpp"
#include "gfx/mem/Image.hpp"
#include "gfx/mem/Allocator.hpp"
#include "gfx/sb/ShaderBindingFactory.hpp"
#include "gfx/sb/ShaderBinding.hpp"
#include <vulkan/vulkan_enums.hpp>

namespace tr::gfx {

   // TODO: figure out a better way to handle the depth image.
   // which pipeline should own it, or should it be shared among them?
   Frame::Frame(const GraphicsDevice& graphicsDevice,
                std::shared_ptr<vk::raii::ImageView> depthImageView,
                sb::ShaderBindingFactory& shaderBindingFactory,
                const std::string_view name)
       : graphicsDevice{graphicsDevice.getVulkanDevice()},
         combinedImageSamplerDescriptorSize{
             graphicsDevice.getDescriptorBufferProperties().combinedImageSamplerDescriptorSize},
         depthImageView{depthImageView},
         shaderBindingFactory{shaderBindingFactory},
         drawExtent{graphicsDevice.DrawImageExtent2D} {

      createSwapchainResources(graphicsDevice);

      // NOLINTNEXTLINE I'd like to init this in the ctor init, but TracyVkContext is a macro
      tracyContext = TracyVkContext(*graphicsDevice.getPhysicalDevice(),
                                    *graphicsDevice.getVulkanDevice(),
                                    *graphicsDevice.getGraphicsQueue(),
                                    *(*commandBuffer));
      TracyVkContextName(tracyContext, name.data(), name.length());

      constexpr auto semaphoreCreateInfo = vk::SemaphoreCreateInfo{};
      constexpr auto fenceCreateInfo =
          vk::FenceCreateInfo{.flags = vk::FenceCreateFlagBits::eSignaled};
      imageAvailableSemaphore =
          std::make_unique<vk::raii::Semaphore>(graphicsDevice.getVulkanDevice(),
                                                semaphoreCreateInfo);
      renderFinishedSemaphore =
          std::make_unique<vk::raii::Semaphore>(graphicsDevice.getVulkanDevice(),
                                                semaphoreCreateInfo);
      inFlightFence =
          std::make_unique<vk::raii::Fence>(graphicsDevice.getVulkanDevice(), fenceCreateInfo);

      // Create an ObjectData buffer
      constexpr auto objectDataBufferCreateInfo =
          vk::BufferCreateInfo{.size = sizeof(ObjectData) * MAX_OBJECTS,
                               .usage = vk::BufferUsageFlagBits::eStorageBuffer |
                                        vk::BufferUsageFlagBits::eShaderDeviceAddress};

      constexpr auto objectDataAllocationCreateInfo =
          vma::AllocationCreateInfo{.usage = vma::MemoryUsage::eCpuToGpu,
                                    .requiredFlags = vk::MemoryPropertyFlagBits::eHostCoherent};

      objectDataBuffer = graphicsDevice.getAllocator().createBuffer(&objectDataBufferCreateInfo,
                                                                    &objectDataAllocationCreateInfo,
                                                                    "Object Data Buffer");
      objectDataBuffer->mapBuffer();

      // create CameraData (PerFrame) buffer
      constexpr auto cameraDataBufferCreateInfo =
          vk::BufferCreateInfo{.size = sizeof(CameraData),
                               .usage = vk::BufferUsageFlagBits::eUniformBuffer |
                                        vk::BufferUsageFlagBits::eShaderDeviceAddress};

      constexpr auto cameraDataAllocationCreateInfo =
          vma::AllocationCreateInfo{.usage = vma::MemoryUsage::eCpuToGpu,
                                    .requiredFlags = vk::MemoryPropertyFlagBits::eHostCoherent};

      cameraDataBuffer = graphicsDevice.getAllocator().createBuffer(&cameraDataBufferCreateInfo,
                                                                    &cameraDataAllocationCreateInfo,
                                                                    "Camera Data Buffer");
      cameraDataBuffer->mapBuffer();

      // Create AnimationDataBuffer
      constexpr auto animationDataBufferCreateInfo =
          vk::BufferCreateInfo{.size = sizeof(AnimationData) * MAX_OBJECTS,
                               .usage = vk::BufferUsageFlagBits::eStorageBuffer |
                                        vk::BufferUsageFlagBits::eShaderDeviceAddress};

      constexpr auto animationDataAllocationCreateInfo =
          vma::AllocationCreateInfo{.usage = vma::MemoryUsage::eCpuToGpu,
                                    .requiredFlags = vk::MemoryPropertyFlagBits::eHostCoherent};

      animationDataBuffer =
          graphicsDevice.getAllocator().createBuffer(&animationDataBufferCreateInfo,
                                                     &animationDataAllocationCreateInfo,
                                                     "Animation Data Buffer");
      // TODO: abstraction over buffers that supports creating them mapped
      animationDataBuffer->mapBuffer();

      // Create PerFrame ShaderBinding
      perFrameShaderBinding =
          shaderBindingFactory.createShaderBinding(sb::ShaderBindingHandle::PerFrame);
      perFrameShaderBinding->bindBuffer(0, *cameraDataBuffer, sizeof(CameraData));

      objectDataShaderBinding =
          shaderBindingFactory.createShaderBinding(sb::ShaderBindingHandle::ObjectData);
      objectDataShaderBinding->bindBuffer(0, *objectDataBuffer, sizeof(ObjectData) * MAX_OBJECTS);

      textureShaderBinding =
          shaderBindingFactory.createShaderBinding(sb::ShaderBindingHandle::Bindless);

      animationDataShaderBinding =
          shaderBindingFactory.createShaderBinding(sb::ShaderBindingHandle::AnimationData);
      animationDataShaderBinding->bindBuffer(0,
                                             *animationDataBuffer,
                                             sizeof(AnimationData) * MAX_OBJECTS);

      const auto drawImageFormat = vk::Format::eR16G16B16A16Sfloat;
      const auto drawImageExtent = graphicsDevice.DrawImageExtent2D;

      const auto imageCreateInfo = vk::ImageCreateInfo{
          .imageType = vk::ImageType::e2D,
          .format = drawImageFormat,
          .extent = vk::Extent3D{drawImageExtent.width, drawImageExtent.height, 1},
          .mipLevels = 1,
          .arrayLayers = 1,
          .samples = vk::SampleCountFlagBits::e1,
          .tiling = vk::ImageTiling::eOptimal,
          .usage = vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst |
                   vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eColorAttachment,
          .sharingMode = vk::SharingMode::eExclusive,
          .initialLayout = vk::ImageLayout::eUndefined};

      const auto imageAllocateCreateInfo =
          vma::AllocationCreateInfo{.usage = vma::MemoryUsage::eGpuOnly,
                                    .requiredFlags = vk::MemoryPropertyFlagBits::eDeviceLocal};
      drawImage = graphicsDevice.getAllocator().createImage(imageCreateInfo,
                                                            imageAllocateCreateInfo,
                                                            "Draw Image");

      const auto imageViewCreateInfo =
          vk::ImageViewCreateInfo{.image = drawImage->getImage(),
                                  .viewType = vk::ImageViewType::e2D,
                                  .format = drawImageFormat,
                                  .subresourceRange = {
                                      .aspectMask = vk::ImageAspectFlagBits::eColor,
                                      .levelCount = 1,
                                      .layerCount = 1,
                                  }};
      drawImageView = std::make_unique<vk::raii::ImageView>(
          graphicsDevice.getVulkanDevice().createImageView(imageViewCreateInfo));
   }

   void Frame::updateTextures(const std::vector<vk::DescriptorImageInfo>& imageInfos) {
      textureShaderBinding->bindImageSamplers(3, imageInfos);
   }

   void Frame::prepareFrame() {
      TracyVkZone(tracyContext, **commandBuffer, "render room");

      Helpers::transitionImage(*commandBuffer,
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
          .clearValue = vk::ClearValue{.depthStencil =
                                           vk::ClearDepthStencilValue{.depth = 1.f, .stencil = 0}},
      };

      const auto renderingInfo =
          vk::RenderingInfo{.renderArea = vk::Rect2D{.offset = {0, 0}, .extent = drawExtent},
                            .layerCount = 1,
                            .colorAttachmentCount = 1,
                            .pColorAttachments = &colorAttachmentInfo,
                            .pDepthAttachment = &depthAttachmentInfo};

      commandBuffer->beginRendering(renderingInfo);
   }

   void Frame::end3D(const vk::Image& swapchainImage, const vk::Extent2D& swapchainExtent) {
      commandBuffer->endRendering();

      Helpers::transitionImage(*commandBuffer,
                               drawImage->getImage(),
                               vk::ImageLayout::eColorAttachmentOptimal,
                               vk::ImageLayout::eTransferSrcOptimal);
      Helpers::transitionImage(*commandBuffer,
                               swapchainImage,
                               vk::ImageLayout::eUndefined,
                               vk::ImageLayout::eTransferDstOptimal);

      Helpers::copyImageToImage(*commandBuffer,
                                drawImage->getImage(),
                                swapchainImage,
                                drawExtent,
                                swapchainExtent);

      Helpers::transitionImage(*commandBuffer,
                               swapchainImage,
                               vk::ImageLayout::eTransferDstOptimal,
                               vk::ImageLayout::eColorAttachmentOptimal);
   }

   void Frame::renderOverlay(const vk::raii::ImageView& swapchainImageView,
                             const vk::Extent2D& swapchainExtent) {
      const auto colorAttachment = vk::RenderingAttachmentInfo{
          .imageView = *swapchainImageView,
          .imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
          .loadOp = vk::AttachmentLoadOp::eLoad,
          .storeOp = vk::AttachmentStoreOp::eStore,
      };

      const auto renderInfo = vk::RenderingInfo{
          .renderArea = vk::Rect2D{.offset = {0, 0}, .extent = swapchainExtent},
          .layerCount = 1,
          .colorAttachmentCount = 1,
          .pColorAttachments = &colorAttachment,
      };

      commandBuffer->beginRendering(renderInfo);

      ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), **commandBuffer);

      commandBuffer->endRendering();
   }

   void Frame::endFrame(const vk::Image& swapchainImage) {
      Helpers::transitionImage(*commandBuffer,
                               swapchainImage,
                               vk::ImageLayout::eColorAttachmentOptimal,
                               vk::ImageLayout::ePresentSrcKHR);

      TracyVkCollect(tracyContext, **commandBuffer);
   }

   const vk::Image& Frame::getDrawImage() const {
      return drawImage->getImage();
   }

   void Frame::updateObjectDataBuffer(const ObjectData* data, const size_t size) {
      this->objectDataBuffer->updateMappedBufferValue(data, size);
   }

   void Frame::updatePerFrameDataBuffer(const CameraData* data, const size_t size) {
      this->cameraDataBuffer->updateMappedBufferValue(data, size);
   }

   void Frame::updateAnimationDataBuffer(const AnimationData* data, const size_t size) {
      this->animationDataBuffer->updateMappedBufferValue(data, size);
   }

   void Frame::destroySwapchainResources() {
      commandBuffer.reset();
   }

   void Frame::createSwapchainResources(const GraphicsDevice& graphicsDevice) {
      const auto allocInfo =
          vk::CommandBufferAllocateInfo{.commandPool = *graphicsDevice.getCommandPool(),
                                        .level = vk::CommandBufferLevel::ePrimary,
                                        .commandBufferCount = 1};
      auto commandBuffers = graphicsDevice.getVulkanDevice().allocateCommandBuffers(allocInfo);
      commandBuffer = std::make_unique<vk::raii::CommandBuffer>(std::move(commandBuffers[0]));
   }

   Frame::~Frame() {
      objectDataBuffer->unmapBuffer();
      TracyVkDestroy(tracyContext);
   }
}