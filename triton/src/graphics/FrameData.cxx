#include "FrameData.hpp"
#include "Logger.hpp"
#include "graphics/pipeline/ObjectMatrices.hpp"
#include "graphics/DebugHelpers.hpp"

FrameData::FrameData(const vk::raii::Device& device,
                     const vk::raii::PhysicalDevice& physicalDevice,
                     const vk::raii::CommandPool& commandPool,
                     const vma::raii::Allocator& raiillocator,
                     const vk::raii::DescriptorPool& descriptorPool,
                     const vk::raii::DescriptorSetLayout& bindlessDescriptorSetLayout,
                     const vk::raii::DescriptorSetLayout& objectDescriptorSetLayout,
                     const vk::raii::DescriptorSetLayout& perFrameDescriptorSetLayout,
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

   // Create an ObjectData buffer
   constexpr auto objectDataBufferCreateInfo = vk::BufferCreateInfo{
       .size = sizeof(ObjectData) * MAX_OBJECTS, .usage = vk::BufferUsageFlagBits::eStorageBuffer};

   constexpr auto objectDataAllocationCreateInfo =
       vma::AllocationCreateInfo{.usage = vma::MemoryUsage::eCpuToGpu,
                                 .requiredFlags = vk::MemoryPropertyFlagBits::eHostCoherent};

   objectDataBuffer = raiillocator.createBuffer(
       &objectDataBufferCreateInfo, &objectDataAllocationCreateInfo, "Object Data Buffer");

   // create cameradata buffer
   constexpr auto cameraDataBufferCreateInfo = vk::BufferCreateInfo{
       .size = sizeof(CameraData), .usage = vk::BufferUsageFlagBits::eUniformBuffer};

   constexpr auto cameraDataAllocationCreateInfo =
       vma::AllocationCreateInfo{.usage = vma::MemoryUsage::eCpuToGpu,
                                 .requiredFlags = vk::MemoryPropertyFlagBits::eHostCoherent};

   cameraDataBuffer = raiillocator.createBuffer(
       &cameraDataBufferCreateInfo, &cameraDataAllocationCreateInfo, "Camera Data Buffer");

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

   // Create the cameradata descriptor set
   const auto cameraDSAllocateInfo =
       vk::DescriptorSetAllocateInfo{.descriptorPool = *descriptorPool,
                                     .descriptorSetCount = 1,
                                     .pSetLayouts = &(*perFrameDescriptorSetLayout)};
   perFrameDescriptorSet = std::make_unique<vk::raii::DescriptorSet>(
       std::move(device.allocateDescriptorSets(cameraDSAllocateInfo).front()));
   graphics::setObjectName(**perFrameDescriptorSet,
                           device,
                           vk::raii::DescriptorSet::debugReportObjectType,
                           "Per Frame Descriptor Set");

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
   const auto perFrameDataBufferInfo = vk::DescriptorBufferInfo{
       .buffer = cameraDataBuffer->getBuffer(), .offset = 0, .range = sizeof(CameraData)};
   const auto perFrameDataDescriptorWrite =
       vk::WriteDescriptorSet{.dstSet = **perFrameDescriptorSet,
                              .dstBinding = 0,
                              .dstArrayElement = 0,
                              .descriptorCount = 1,
                              .descriptorType = vk::DescriptorType::eUniformBuffer,
                              .pBufferInfo = &perFrameDataBufferInfo};

   const auto writes = std::array{objectDataDescriptorWrite, perFrameDataDescriptorWrite};

   device.updateDescriptorSets(writes, nullptr);
}

void FrameData::updateObjectDataBuffer(ObjectData* data, const size_t size) {
   this->objectDataBuffer->updateBufferValue(data, size);
}

FrameData::~FrameData() {
   TracyVkDestroy(tracyContext);
}