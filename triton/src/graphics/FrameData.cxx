#include "FrameData.hpp"
#include "graphics/pipeline/ObjectMatrices.hpp"

using Core::Log;

FrameData::FrameData(const vk::raii::Device& device,
                     const vk::raii::CommandPool& commandPool,
                     const vma::raii::Allocator& raiillocator,
                     const vk::raii::DescriptorPool& descriptorPool,
                     const vk::raii::DescriptorSetLayout& descriptorSetLayout,
                     const vk::DescriptorImageInfo textureImageInfo) {

   const auto allocInfo = vk::CommandBufferAllocateInfo{.commandPool = *commandPool,
                                                        .level = vk::CommandBufferLevel::ePrimary,
                                                        .commandBufferCount = 1};
   auto commandBuffers = device.allocateCommandBuffers(allocInfo);

   constexpr auto semaphoreCreateInfo = vk::SemaphoreCreateInfo{};
   constexpr auto fenceCreateInfo =
       vk::FenceCreateInfo{.flags = vk::FenceCreateFlagBits::eSignaled};

   commandBuffer = std::make_unique<vk::raii::CommandBuffer>(std::move(commandBuffers[0]));
   imageAvailableSemaphore = std::make_unique<vk::raii::Semaphore>(device, semaphoreCreateInfo);
   renderFinishedSemaphore = std::make_unique<vk::raii::Semaphore>(device, semaphoreCreateInfo);
   inFlightFence = std::make_unique<vk::raii::Fence>(device, fenceCreateInfo);

   constexpr auto bufferCreateInfo = vk::BufferCreateInfo{
       .size = sizeof(ObjectMatrices), .usage = vk::BufferUsageFlagBits::eUniformBuffer};

   constexpr auto allocationCreateInfo =
       vma::AllocationCreateInfo{.usage = vma::MemoryUsage::eCpuToGpu,
                                 .requiredFlags = vk::MemoryPropertyFlagBits::eHostCoherent};

   objectMatricesBuffer = raiillocator.createBuffer(
       &bufferCreateInfo, &allocationCreateInfo, "Object Matrices Buffer");

   auto objectMatrices =
       ObjectMatrices{.model = glm::mat4{1.f}, .view = glm::mat4{1.f}, .proj = glm::mat4{1.f}};

   const auto descriptorSetAllocateInfo =
       vk::DescriptorSetAllocateInfo{.descriptorPool = *descriptorPool,
                                     .descriptorSetCount = 1,
                                     .pSetLayouts = &(*descriptorSetLayout)};

   descriptorSet = std::make_unique<vk::raii::DescriptorSet>(
       std::move(device.allocateDescriptorSets(descriptorSetAllocateInfo).front()));

   const auto objectMatricesBufferInfo = vk::DescriptorBufferInfo{
       .buffer = objectMatricesBuffer->getBuffer(), .offset = 0, .range = sizeof(ObjectMatrices)};

   const auto objectMatricesDescriptorWrite = vk::WriteDescriptorSet{
       .dstSet = **descriptorSet,
       .dstBinding = 0,
       .dstArrayElement = 0,
       .descriptorCount = 1,
       .descriptorType = vk::DescriptorType::eUniformBuffer,
       .pBufferInfo = &objectMatricesBufferInfo,
   };

   const auto textureDescriptorWrite = vk::WriteDescriptorSet{
       .dstSet = **descriptorSet,
       .dstBinding = 1,
       .dstArrayElement = 0,
       .descriptorCount = 1,
       .descriptorType = vk::DescriptorType::eCombinedImageSampler,
       .pImageInfo = &textureImageInfo,
   };

   const auto writes = std::array{objectMatricesDescriptorWrite, textureDescriptorWrite};

   device.updateDescriptorSets(writes, nullptr);

   const auto dest = raiillocator.mapMemory(*objectMatricesBuffer);
   memcpy(dest, &objectMatrices, sizeof(ObjectMatrices));
   raiillocator.unmapMemory(*objectMatricesBuffer);
}
