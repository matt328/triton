#include "FrameData.hpp"
#include "ObjectData.hpp"
#include "GraphicsDevice.hpp"
#include "helpers/Vulkan.hpp"

namespace Triton::Graphics {

   FrameData::FrameData(const GraphicsDevice& graphicsDevice,
                        const vk::raii::DescriptorSetLayout& bindlessDescriptorSetLayout,
                        const vk::raii::DescriptorSetLayout& objectDescriptorSetLayout,
                        const vk::raii::DescriptorSetLayout& perFrameDescriptorSetLayout,
                        const std::string_view name) {

      const auto allocInfo =
          vk::CommandBufferAllocateInfo{.commandPool = *graphicsDevice.getCommandPool(),
                                        .level = vk::CommandBufferLevel::ePrimary,
                                        .commandBufferCount = 1};

      auto commandBuffers = graphicsDevice.getVulkanDevice().allocateCommandBuffers(allocInfo);

      constexpr auto semaphoreCreateInfo = vk::SemaphoreCreateInfo{};
      constexpr auto fenceCreateInfo =
          vk::FenceCreateInfo{.flags = vk::FenceCreateFlagBits::eSignaled};

      commandBuffer = std::make_unique<vk::raii::CommandBuffer>(std::move(commandBuffers[0]));

      // NOLINTNEXTLINE I'd like to init this in the ctor init, but TracyVkContext is a macro
      tracyContext = TracyVkContext(*graphicsDevice.getPhysicalDevice(),
                                    *graphicsDevice.getVulkanDevice(),
                                    *graphicsDevice.getGraphicsQueue(),
                                    *(*commandBuffer));
      TracyVkContextName(tracyContext, name.data(), name.length());

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
   }

   void FrameData::updateObjectDataBuffer(const ObjectData* data, const size_t size) {
      this->objectDataBuffer->updateBufferValue(data, size);
   }

   FrameData::~FrameData() {
      TracyVkDestroy(tracyContext);
   }
}