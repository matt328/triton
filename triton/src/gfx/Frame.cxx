#include "Frame.hpp"
#include "gfx/ObjectData.hpp"
#include "GraphicsDevice.hpp"
#include "gfx/ds/Layout.hpp"
#include "gfx/ds/LayoutFactory.hpp"
#include "gfx/helpers/Rendering.hpp"
#include "gfx/mem/Buffer.hpp"
#include "gfx/mem/Image.hpp"
#include "gfx/mem/Allocator.hpp"
#include <vulkan/vulkan_enums.hpp>
#include <vulkan/vulkan_structs.hpp>

namespace tr::gfx {

   // TODO: figure out a better way to handle the depth image.
   // which pipeline should own it, or should it be shared among them?
   Frame::Frame(const GraphicsDevice& graphicsDevice,
                std::shared_ptr<vk::raii::ImageView> depthImageView,
                ds::LayoutFactory& layoutFactory,
                const std::string_view name)
       : graphicsDevice{graphicsDevice.getVulkanDevice()},
         combinedImageSamplerDescriptorSize{
             graphicsDevice.getDescriptorBufferProperties().combinedImageSamplerDescriptorSize},
         depthImageView{depthImageView},
         layoutFactory{layoutFactory},
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

      // Create Per Frame Descriptor Buffer
      const auto& perFrameLayout = layoutFactory.getLayout(ds::LayoutHandle::PerFrame);
      const auto size = perFrameLayout.getAlignedSize();
      perFrameDescriptorBuffer = graphicsDevice.getAllocator().createDescriptorBuffer(size);

      // Set CameraDataBuffer's Address in perFrameDescriptorBuffer
      const auto ai =
          vk::DescriptorAddressInfoEXT{.address = cameraDataBuffer->getDeviceAddress(),
                                       .range = cameraDataBuffer->getBufferInfo()->range};
      const auto bdi = vk::DescriptorGetInfoEXT{
          .type = vk::DescriptorType::eUniformBuffer,
          .data = {.pUniformBuffer = &ai},
      };

      const auto uniformBufferSize =
          graphicsDevice.getDescriptorBufferProperties().uniformBufferDescriptorSize;

      graphicsDevice.getVulkanDevice().getDescriptorEXT(bdi,
                                                        uniformBufferSize,
                                                        perFrameDescriptorBuffer->getData());

      // Create Object Data Descriptor Buffer
      const auto& objectDataLayout = layoutFactory.getLayout(ds::LayoutHandle::ObjectData);
      const auto objectDataSize = objectDataLayout.getAlignedSize();
      objectDataDescriptorBuffer =
          graphicsDevice.getAllocator().createDescriptorBuffer(objectDataSize);

      // Set ObjectDataBuffer's address in descriptor buffer
      // I think this can be done once since the descriptor itself doesn't change, only the contents
      // of the buffer bound to it
      const auto addressInfo =
          vk::DescriptorAddressInfoEXT{.address = objectDataBuffer->getDeviceAddress(),
                                       .range = objectDataBuffer->getBufferInfo()->range};
      const auto descriptorGetInfo =
          vk::DescriptorGetInfoEXT{.type = vk::DescriptorType::eStorageBuffer,
                                   .data = {.pStorageBuffer = &addressInfo}};
      const auto storageBufferSize =
          graphicsDevice.getDescriptorBufferProperties().storageBufferDescriptorSize;

      graphicsDevice.getVulkanDevice().getDescriptorEXT(descriptorGetInfo,
                                                        storageBufferSize,
                                                        objectDataDescriptorBuffer->getData());

      // Create Texture Descriptor Buffer
      /*
         A Descriptor can be thought of as a definition of a slot that the shaders can specify as
         their input params.

         With a 'bindless' DescriptorSet containing multiple descriptors, getAlignedSize takes this
         into account and we allocate a single buffer holding all the descriptors, one for each
         texture.

         We don't need to keep setting the offset into the descriptor buffer on the CPU side since
         we index into the buffer on the GPU side with the ObjectData handles. Setting offsets per
         draw call seems to be an escape hatch to support non-bindless (bindful?) rendering.
         For now, we'll pass in an integer offset into that list as part of the
         ObjectData. Eventually we should replace ObjectData's handles with BufferDeviceAddresses.
      */
      const auto& textureLayout = layoutFactory.getLayout(ds::LayoutHandle::Bindless);
      const auto textureLayoutSize = textureLayout.getAlignedSize();
      textureDescriptorBuffer =
          graphicsDevice.getAllocator().createDescriptorBuffer(textureLayoutSize);

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
      const auto& textureLayout = layoutFactory.getLayout(ds::LayoutHandle::Bindless);
      const auto textureLayoutSize = textureLayout.getAlignedSize();

      auto dataPtr = static_cast<char*>(textureDescriptorBuffer->getData());
      auto i = 0;
      for (const auto imageInfo : imageInfos) {
         const auto offset = (i * textureLayoutSize) + textureLayout.getBindingOffset(0);
         const auto imageDescriptorInfo =
             vk::DescriptorGetInfoEXT{.type = vk::DescriptorType::eCombinedImageSampler,
                                      .data = {.pCombinedImageSampler = &imageInfo}};
         // This function's name is odd, I think it gets a descriptor representing the
         // imageDescriptorInfo.data, then writes it to the memory address given by the last
         // parameter?
         // Also there is almost certainly something wrong with this pointer arithmetic I really
         // doubt  would have yolo'd that on the first try
         graphicsDevice.getDescriptorEXT(imageDescriptorInfo,
                                         combinedImageSamplerDescriptorSize,
                                         dataPtr + offset);
         i++;
      }
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