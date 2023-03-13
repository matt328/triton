#include "FrameData.h"

#include "DefaultPipeline.h"

#include <vulkan/vulkan_raii.hpp>

#include "ObjectMatrices.h"

FrameData::FrameData(const vk::raii::Device& device,
                     const vk::raii::CommandPool& commandPool,
                     const vma::raii::Allocator& raiillocator,
                     const vk::raii::DescriptorPool& descriptorPool) {

   const auto allocInfo = vk::CommandBufferAllocateInfo{.commandPool = *commandPool,
                                                        .level = vk::CommandBufferLevel::ePrimary,
                                                        .commandBufferCount = 1};
   auto commandBuffers = device.allocateCommandBuffers(allocInfo);

   constexpr auto semaphoreCreateInfo = vk::SemaphoreCreateInfo{};
   constexpr auto fenceCreateInfo =
       vk::FenceCreateInfo{.flags = vk::FenceCreateFlagBits::eSignaled};

   constexpr auto bufferCreateInfo = vk::BufferCreateInfo{
       .size = sizeof(ObjectMatrices), .usage = vk::BufferUsageFlagBits::eUniformBuffer};

   constexpr auto allocationCreateInfo =
       vma::AllocationCreateInfo{.usage = vma::MemoryUsage::eCpuToGpu};

   commandBuffer = std::make_unique<vk::raii::CommandBuffer>(std::move(commandBuffers[0]));
   imageAvailableSemaphore = std::make_unique<vk::raii::Semaphore>(device, semaphoreCreateInfo);
   renderFinishedSemaphore = std::make_unique<vk::raii::Semaphore>(device, semaphoreCreateInfo);
   inFlightFence = std::make_unique<vk::raii::Fence>(device, fenceCreateInfo);

   objectMatricesBuffer = raiillocator.createBuffer(
       &bufferCreateInfo, &allocationCreateInfo, std::format("Object Matrices Buffer"));

   this->descriptorSetLayout = std::make_unique<vk::raii::DescriptorSetLayout>(
       DefaultPipeline::createDescriptorSetLayout(device));

   const auto descriptorSetAllocateInfo =
       vk::DescriptorSetAllocateInfo{.descriptorPool = *descriptorPool,
                                     .descriptorSetCount = 1,
                                     .pSetLayouts = &(**descriptorSetLayout)};

   descriptorSet = std::make_unique<vk::raii::DescriptorSet>(
       std::move(device.allocateDescriptorSets(descriptorSetAllocateInfo).front()));

   const auto objectMatricesBufferInfo = vk::DescriptorBufferInfo{
       .buffer = objectMatricesBuffer->getBuffer(), .offset = 0, .range = sizeof(ObjectMatrices)};

   // const auto textureImageInfo = vk::DescriptorImageInfo{
   //     .sampler = *textureSampler,
   //     .imageView = *(*textureImageView),
   //     .imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal,
   // };
}

FrameData::~FrameData() {
   Log::core->debug("FrameData Destructor");
}
