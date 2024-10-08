#include "Frame.hpp"
#include "cm/ObjectData.hpp"
#include "GraphicsDevice.hpp"
#include "helpers/Rendering.hpp"
#include "mem/Buffer.hpp"
#include "mem/Image.hpp"
#include "mem/Allocator.hpp"
#include "sb/ShaderBindingFactory.hpp"
#include "sb/ShaderBinding.hpp"
#include <utility>
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
         depthImageView{std::move(depthImageView)},
         shaderBindingFactory{shaderBindingFactory},
         drawExtent{GraphicsDevice::DrawImageExtent2D} {

      createSwapchainResources(graphicsDevice);

      // NOLINTNEXTLINE I'd like to init this in the ctor init, but TracyVkContext is a macro
      tracyContext = TracyVkContext(*graphicsDevice.getPhysicalDevice(),
                                    *graphicsDevice.getVulkanDevice(),
                                    *graphicsDevice.getGraphicsQueue(),
                                    **commandBuffer);
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
          vk::BufferCreateInfo{.size = sizeof(cm::gpu::ObjectData) * cm::gpu::MAX_OBJECTS,
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
          vk::BufferCreateInfo{.size = sizeof(cm::gpu::CameraData),
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
          vk::BufferCreateInfo{.size = sizeof(cm::AnimationData) * cm::gpu::MAX_OBJECTS,
                               .usage = vk::BufferUsageFlagBits::eStorageBuffer |
                                        vk::BufferUsageFlagBits::eShaderDeviceAddress};

      constexpr auto animationDataAllocationCreateInfo =
          vma::AllocationCreateInfo{.usage = vma::MemoryUsage::eCpuToGpu,
                                    .requiredFlags = vk::MemoryPropertyFlagBits::eHostCoherent};

      animationDataBuffer =
          graphicsDevice.getAllocator().createBuffer(&animationDataBufferCreateInfo,
                                                     &animationDataAllocationCreateInfo,
                                                     "Animation Data Buffer");
      animationDataBuffer->mapBuffer();

      // Create PerFrame ShaderBinding
      perFrameShaderBinding =
          shaderBindingFactory.createShaderBinding(sb::ShaderBindingHandle::PerFrame);
      perFrameShaderBinding->bindBuffer(0, *cameraDataBuffer, sizeof(cm::gpu::CameraData));

      objectDataShaderBinding =
          shaderBindingFactory.createShaderBinding(sb::ShaderBindingHandle::ObjectData);
      objectDataShaderBinding->bindBuffer(0,
                                          *objectDataBuffer,
                                          sizeof(cm::gpu::ObjectData) * cm::gpu::MAX_OBJECTS);

      textureShaderBinding =
          shaderBindingFactory.createShaderBinding(sb::ShaderBindingHandle::Bindless);

      animationDataShaderBinding =
          shaderBindingFactory.createShaderBinding(sb::ShaderBindingHandle::AnimationData);
      animationDataShaderBinding->bindBuffer(0,
                                             *animationDataBuffer,
                                             sizeof(cm::AnimationData) * cm::gpu::MAX_OBJECTS);

      constexpr auto drawImageFormat = vk::Format::eR16G16B16A16Sfloat;
      const auto [width, height] = GraphicsDevice::DrawImageExtent2D;

      const auto imageCreateInfo = vk::ImageCreateInfo{
          .imageType = vk::ImageType::e2D,
          .format = drawImageFormat,
          .extent = vk::Extent3D{width, height, 1},
          .mipLevels = 1,
          .arrayLayers = 1,
          .samples = vk::SampleCountFlagBits::e1,
          .tiling = vk::ImageTiling::eOptimal,
          .usage = vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst |
                   vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eColorAttachment,
          .sharingMode = vk::SharingMode::eExclusive,
          .initialLayout = vk::ImageLayout::eUndefined};

      constexpr auto imageAllocateCreateInfo =
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

   void Frame::updateTextures(const std::vector<vk::DescriptorImageInfo>& imageInfos) const {
      textureShaderBinding->bindImageSamplers(3, imageInfos);
   }

   void Frame::prepareFrame() const {

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

   void Frame::end3D(const vk::Image& swapchainImage, const vk::Extent2D& swapchainExtent) const {
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
                             const vk::Extent2D& swapchainExtent) const {
      ZoneNamedN(imguiZone, "Render ImGui", true);
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

   void Frame::endFrame(const vk::Image& swapchainImage) const {
      Helpers::transitionImage(*commandBuffer,
                               swapchainImage,
                               vk::ImageLayout::eColorAttachmentOptimal,
                               vk::ImageLayout::ePresentSrcKHR);

      TracyVkCollect(tracyContext, **commandBuffer);
   }

   auto Frame::getDrawImage() const -> const vk::Image& {
      return drawImage->getImage();
   }

   void Frame::updateObjectDataBuffer(const cm::gpu::ObjectData* data, const size_t size) const {
      this->objectDataBuffer->updateMappedBufferValue(data, size);
   }

   void Frame::updatePerFrameDataBuffer(const cm::gpu::CameraData* data, const size_t size) const {
      this->cameraDataBuffer->updateMappedBufferValue(data, size);
   }

   void Frame::updateAnimationDataBuffer(const cm::gpu::AnimationData* data,
                                         const size_t size) const {
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