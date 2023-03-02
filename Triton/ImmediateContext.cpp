#pragma once

#include "ImmediateContext.h"
#include "Log.h"

#include <vulkan/vulkan_raii.hpp>

ImmediateContext::ImmediateContext(const std::unique_ptr<vk::raii::Device>& device,
                                   const std::shared_ptr<vk::raii::Queue> newQueue,
                                   const uint32_t queueFamily)
    : queue(newQueue) {
   // Create Command Pool
   const vk::CommandPoolCreateInfo transferCommandPoolCreateInfo{.queueFamilyIndex = queueFamily};

   commandPool = std::make_unique<vk::raii::CommandPool>(
       device->createCommandPool(transferCommandPoolCreateInfo));

   // Create Command Buffer
   const auto uploadAllocInfo =
       vk::CommandBufferAllocateInfo{.commandPool = *(*commandPool),
                                     .level = vk::CommandBufferLevel::ePrimary,
                                     .commandBufferCount = 1};

   commandBuffer = std::make_unique<vk::raii::CommandBuffer>(
       std::move(device->allocateCommandBuffers(uploadAllocInfo)[0]));

   // Create Fence
   fence = std::make_unique<vk::raii::Fence>(device->createFence(vk::FenceCreateInfo{}));
}

void ImmediateContext::submit(std::function<void(vk::raii::CommandBuffer& cmd)>&& fn,
                              const std::unique_ptr<vk::raii::Device>& device) const {
   constexpr vk::CommandBufferBeginInfo cmdBeginInfo{
       .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit};

   commandBuffer->begin(cmdBeginInfo);

   fn(*commandBuffer);

   commandBuffer->end();

   const auto submitInfo =
       vk::SubmitInfo{.commandBufferCount = 1, .pCommandBuffers = &(*(*commandBuffer))};

   queue->submit(submitInfo, *(*fence));

   if (const auto result = device->waitForFences(*(*fence), true, UINT64_MAX);
       result != vk::Result::eSuccess) {
      Log::core->warn("During Immediate Submit, timeout waiting for fence");
   }
   device->resetFences(*(*fence));
   commandPool->reset();
}
