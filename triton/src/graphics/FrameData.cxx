#include "FrameData.hpp"
#include "graphics/RenderDevice.hpp"
#include "graphics/pipeline/ObjectMatrices.hpp"
#include "graphics/DebugHelpers.hpp"

#include <vulkan-memory-allocator-hpp/vk_mem_alloc_structs.hpp>
#include <vulkan/vulkan_enums.hpp>
#include <vulkan/vulkan_raii.hpp>
#include <vulkan/vulkan_structs.hpp>

using Core::Log;

FrameData::FrameData(const vk::raii::Device& device,
                     const vk::raii::PhysicalDevice& physicalDevice,
                     const vk::raii::CommandPool& commandPool,
                     const vma::raii::Allocator& raiillocator,
                     const vk::raii::DescriptorPool& descriptorPool,
                     const vk::raii::DescriptorSetLayout& descriptorSetLayout,
                     const vk::raii::DescriptorSetLayout& bindlessDescriptorSetLayout,
                     const vk::raii::DescriptorSetLayout& objectDescriptorSetLayout,
                     const vk::raii::Queue& queue,
                     const std::string_view name) {

   const auto allocInfo = vk::CommandBufferAllocateInfo{.commandPool = *commandPool,
                                                        .level = vk::CommandBufferLevel::ePrimary,
                                                        .commandBufferCount = 1};
   auto commandBuffers = device.allocateCommandBuffers(allocInfo);

   constexpr auto semaphoreCreateInfo = vk::SemaphoreCreateInfo{};
   constexpr auto fenceCreateInfo =
       vk::FenceCreateInfo{.flags = vk::FenceCreateFlagBits::eSignaled};

   commandBuffer = std::make_unique<vk::raii::CommandBuffer>(std::move(commandBuffers[0]));

   // NOLINTNEXTLINE I'd like to init this in the ctor init, but TracyVkContext is a macro
   tracyContext = TracyVkContext((*physicalDevice), (*device), *queue, *(*commandBuffer));
   TracyVkContextName(tracyContext, name.data(), name.length());

   imageAvailableSemaphore = std::make_unique<vk::raii::Semaphore>(device, semaphoreCreateInfo);
   renderFinishedSemaphore = std::make_unique<vk::raii::Semaphore>(device, semaphoreCreateInfo);
   inFlightFence = std::make_unique<vk::raii::Fence>(device, fenceCreateInfo);

   // Create ObjectMatrices buffer
   constexpr auto bufferCreateInfo = vk::BufferCreateInfo{
       .size = sizeof(ObjectMatrices), .usage = vk::BufferUsageFlagBits::eUniformBuffer};

   constexpr auto allocationCreateInfo =
       vma::AllocationCreateInfo{.usage = vma::MemoryUsage::eCpuToGpu,
                                 .requiredFlags = vk::MemoryPropertyFlagBits::eHostCoherent};

   objectMatricesBuffer = raiillocator.createBuffer(
       &bufferCreateInfo, &allocationCreateInfo, "Object Matrices Buffer");

   // Create an ObjectData buffer
   constexpr auto objectDataBufferCreateInfo = vk::BufferCreateInfo{
       .size = sizeof(ObjectData) * MAX_OBJECTS, .usage = vk::BufferUsageFlagBits::eStorageBuffer};

   constexpr auto objectDataAllocationCreateInfo =
       vma::AllocationCreateInfo{.usage = vma::MemoryUsage::eCpuToGpu,
                                 .requiredFlags = vk::MemoryPropertyFlagBits::eHostCoherent};

   objectDataBuffer = raiillocator.createBuffer(
       &objectDataBufferCreateInfo, &allocationCreateInfo, "Object Data Buffer");

   // create data for objectMatrices
   auto objectMatrices =
       ObjectMatrices{.model = glm::mat4{1.f}, .view = glm::mat4{1.f}, .proj = glm::mat4{1.f}};

   const auto objectDSAllocateInfo =
       vk::DescriptorSetAllocateInfo{.descriptorPool = *descriptorPool,
                                     .descriptorSetCount = 1,
                                     .pSetLayouts = &(*objectDescriptorSetLayout)};

   objectDescriptorSet = std::make_unique<vk::raii::DescriptorSet>(
       std::move(device.allocateDescriptorSets(objectDSAllocateInfo).front()));
   graphics::setObjectName(**objectDescriptorSet,
                           device,
                           vk::raii::DescriptorSet::debugReportObjectType,
                           "Object Descriptor Set");

   // Create the bindless descriptor set
   const auto bindlessDescriptorSetAllocateInfo =
       vk::DescriptorSetAllocateInfo{.descriptorPool = *descriptorPool,
                                     .descriptorSetCount = 1,
                                     .pSetLayouts = &(*bindlessDescriptorSetLayout)};

   bindlessDescriptorSet = std::make_unique<vk::raii::DescriptorSet>(
       std::move(device.allocateDescriptorSets(bindlessDescriptorSetAllocateInfo).front()));

   graphics::setObjectName(**bindlessDescriptorSet,
                           device,
                           vk::raii::DescriptorSet::debugReportObjectType,
                           "Bindless Descriptor Set");

   const auto descriptorSetAllocateInfo =
       vk::DescriptorSetAllocateInfo{.descriptorPool = *descriptorPool,
                                     .descriptorSetCount = 1,
                                     .pSetLayouts = &(*descriptorSetLayout)};

   descriptorSet = std::make_unique<vk::raii::DescriptorSet>(
       std::move(device.allocateDescriptorSets(descriptorSetAllocateInfo).front()));

   // Create a write for the objectMatrices descriptor
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

   const auto writes = std::array{objectMatricesDescriptorWrite, objectDataDescriptorWrite};

   device.updateDescriptorSets(writes, nullptr);

   const auto dest = raiillocator.mapMemory(*objectMatricesBuffer);
   memcpy(dest, &objectMatrices, sizeof(ObjectMatrices));
   raiillocator.unmapMemory(*objectMatricesBuffer);
}

void FrameData::updateObjectDataBuffer(ObjectData* data, const size_t size) {
   this->objectDataBuffer->updateBufferValue(data, size);
}

FrameData::~FrameData() {
   TracyVkDestroy(tracyContext);
}