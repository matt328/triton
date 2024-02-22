#include "FrameData.hpp"
#include "ObjectData.hpp"
#include "GraphicsDevice.hpp"
#include "helpers/Vulkan.hpp"
#include "vma_raii.hpp"

namespace Triton::Graphics {

   FrameData::FrameData(const GraphicsDevice& graphicsDevice,
                        const vk::raii::DescriptorSetLayout& bindlessDescriptorSetLayout,
                        const vk::raii::DescriptorSetLayout& objectDescriptorSetLayout,
                        const vk::raii::DescriptorSetLayout& perFrameDescriptorSetLayout,
                        const std::string_view name) {

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
                               .usage = vk::BufferUsageFlagBits::eStorageBuffer};

      constexpr auto objectDataAllocationCreateInfo =
          vma::AllocationCreateInfo{.usage = vma::MemoryUsage::eCpuToGpu,
                                    .requiredFlags = vk::MemoryPropertyFlagBits::eHostCoherent};

      objectDataBuffer = graphicsDevice.getAllocator().createBuffer(&objectDataBufferCreateInfo,
                                                                    &objectDataAllocationCreateInfo,
                                                                    "Object Data Buffer");

      // create cameradata buffer
      constexpr auto cameraDataBufferCreateInfo =
          vk::BufferCreateInfo{.size = sizeof(CameraData),
                               .usage = vk::BufferUsageFlagBits::eUniformBuffer};

      constexpr auto cameraDataAllocationCreateInfo =
          vma::AllocationCreateInfo{.usage = vma::MemoryUsage::eCpuToGpu,
                                    .requiredFlags = vk::MemoryPropertyFlagBits::eHostCoherent};

      cameraDataBuffer = graphicsDevice.getAllocator().createBuffer(&cameraDataBufferCreateInfo,
                                                                    &cameraDataAllocationCreateInfo,
                                                                    "Camera Data Buffer");

      const auto objectDSAllocateInfo =
          vk::DescriptorSetAllocateInfo{.descriptorPool = *graphicsDevice.getDescriptorPool(),
                                        .descriptorSetCount = 1,
                                        .pSetLayouts = &(*objectDescriptorSetLayout)};

      objectDescriptorSet = std::make_unique<vk::raii::DescriptorSet>(std::move(
          graphicsDevice.getVulkanDevice().allocateDescriptorSets(objectDSAllocateInfo).front()));
      Helpers::setObjectName(**objectDescriptorSet,
                             graphicsDevice.getVulkanDevice(),
                             vk::raii::DescriptorSet::debugReportObjectType,
                             "Object Descriptor Set");

      // Create the cameradata descriptor set
      const auto cameraDSAllocateInfo =
          vk::DescriptorSetAllocateInfo{.descriptorPool = *graphicsDevice.getDescriptorPool(),
                                        .descriptorSetCount = 1,
                                        .pSetLayouts = &(*perFrameDescriptorSetLayout)};
      perFrameDescriptorSet = std::make_unique<vk::raii::DescriptorSet>(std::move(
          graphicsDevice.getVulkanDevice().allocateDescriptorSets(cameraDSAllocateInfo).front()));
      Helpers::setObjectName(**perFrameDescriptorSet,
                             graphicsDevice.getVulkanDevice(),
                             vk::raii::DescriptorSet::debugReportObjectType,
                             "Per Frame Descriptor Set");

      // Create the bindless descriptor set
      const auto bindlessDescriptorSetAllocateInfo =
          vk::DescriptorSetAllocateInfo{.descriptorPool = *graphicsDevice.getDescriptorPool(),
                                        .descriptorSetCount = 1,
                                        .pSetLayouts = &(*bindlessDescriptorSetLayout)};

      bindlessDescriptorSet = std::make_unique<vk::raii::DescriptorSet>(
          std::move(graphicsDevice.getVulkanDevice()
                        .allocateDescriptorSets(bindlessDescriptorSetAllocateInfo)
                        .front()));

      Helpers::setObjectName(**bindlessDescriptorSet,
                             graphicsDevice.getVulkanDevice(),
                             vk::raii::DescriptorSet::debugReportObjectType,
                             "Bindless Descriptor Set");

      // create a write for the objectData descriptor
      const auto objectDataBufferInfo =
          vk::DescriptorBufferInfo{.buffer = objectDataBuffer->getBuffer(),
                                   .offset = 0,
                                   .range = sizeof(ObjectData) * MAX_OBJECTS};

      const auto objectDataDescriptorWrite =
          vk::WriteDescriptorSet{.dstSet = **objectDescriptorSet,
                                 .dstBinding = 0,
                                 .dstArrayElement = 0,
                                 .descriptorCount = 1,
                                 .descriptorType = vk::DescriptorType::eStorageBuffer,
                                 .pBufferInfo = &objectDataBufferInfo};

      // create a write for the cameradata descriptor
      const auto perFrameDataBufferInfo =
          vk::DescriptorBufferInfo{.buffer = cameraDataBuffer->getBuffer(),
                                   .offset = 0,
                                   .range = sizeof(CameraData)};
      const auto perFrameDataDescriptorWrite =
          vk::WriteDescriptorSet{.dstSet = **perFrameDescriptorSet,
                                 .dstBinding = 0,
                                 .dstArrayElement = 0,
                                 .descriptorCount = 1,
                                 .descriptorType = vk::DescriptorType::eUniformBuffer,
                                 .pBufferInfo = &perFrameDataBufferInfo};

      const auto writes = std::array{objectDataDescriptorWrite, perFrameDataDescriptorWrite};

      graphicsDevice.getVulkanDevice().updateDescriptorSets(writes, nullptr);

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

   const vk::Image& FrameData::getDrawImage() const {
      return drawImage->getImage();
   }

   void FrameData::updateObjectDataBuffer(const ObjectData* data, const size_t size) {
      this->objectDataBuffer->updateBufferValue(data, size);
   }

   void FrameData::destroySwapchainResources() {
      commandBuffer.reset();
   }

   void FrameData::createSwapchainResources(const GraphicsDevice& graphicsDevice) {
      const auto allocInfo =
          vk::CommandBufferAllocateInfo{.commandPool = *graphicsDevice.getCommandPool(),
                                        .level = vk::CommandBufferLevel::ePrimary,
                                        .commandBufferCount = 1};
      auto commandBuffers = graphicsDevice.getVulkanDevice().allocateCommandBuffers(allocInfo);
      commandBuffer = std::make_unique<vk::raii::CommandBuffer>(std::move(commandBuffers[0]));
   }

   FrameData::~FrameData() {
      TracyVkDestroy(tracyContext);
   }
}