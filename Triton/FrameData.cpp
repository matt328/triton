#include "FrameData.h"

#include <vulkan/vulkan_raii.hpp>

#include "ObjectMatrices.h"

FrameData::FrameData(const vk::raii::Device& device,
                     const vk::raii::CommandPool& commandPool,
                     const vma::raii::Allocator& raiillocator) {
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
}

FrameData::~FrameData() {
   Log::core->debug("FrameData Destructor");
}
