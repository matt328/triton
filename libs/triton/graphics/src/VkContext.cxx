#include "VkContext.hpp"

#include "helpers/Vulkan.hpp"

namespace tr::gfx {

   VkContext::VkContext(const vk::raii::Device& device,
                        const vk::raii::PhysicalDevice& physicalDevice,
                        const uint32_t queueIndex,
                        const uint32_t queueFamily,
                        const std::string_view& name)
       : device{device},
         physicalDevice{physicalDevice},
         queue{std::make_unique<vk::raii::Queue>(device.getQueue(queueFamily, queueIndex))} {

      Helpers::setObjectName(**queue, device, name);
      Log.trace("Created Transfer Queue");

      // Create Command Pool
      const auto poolCreateInfo = vk::CommandPoolCreateInfo{
          .flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
          .queueFamilyIndex = queueFamily,
      };

      commandPool =
          std::make_unique<vk::raii::CommandPool>(device.createCommandPool(poolCreateInfo));

      // Create Command Buffer
      const auto cmdAllocInfo =
          vk::CommandBufferAllocateInfo{.commandPool = **commandPool,
                                        .level = vk::CommandBufferLevel::ePrimary,
                                        .commandBufferCount = 1};

      commandBuffer = std::make_unique<vk::raii::CommandBuffer>(
          std::move(device.allocateCommandBuffers(cmdAllocInfo)[0]));

      tracyContext = TracyVkContext((*physicalDevice), (*device), **queue, *(*commandBuffer));

      TracyVkContextName(tracyContext, name.data(), name.length());

      // Create Fence
      fence = std::make_unique<vk::raii::Fence>(device.createFence(vk::FenceCreateInfo{}));
   }

   VkContext::~VkContext() {
      TracyVkDestroy(tracyContext);
   }

   void VkContext::submit(std::function<void(vk::raii::CommandBuffer& cmd)>&& fn) const {
      constexpr vk::CommandBufferBeginInfo cmdBeginInfo{
          .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit};

      commandBuffer->begin(cmdBeginInfo);
      {
         TracyVkZone(tracyContext, **commandBuffer, "immediate submit");
         fn(*commandBuffer);
         TracyVkCollect(tracyContext, **commandBuffer);
      }
      commandBuffer->end();

      const auto submitInfo =
          vk::SubmitInfo{.commandBufferCount = 1, .pCommandBuffers = &(**commandBuffer)};

      queue->submit(submitInfo, **fence);

      if (const auto result = device.waitForFences(**fence, true, UINT64_MAX);
          result != vk::Result::eSuccess) {
         Log.warn("Timeout waiting for fence during immediate submit");
      }
      device.resetFences(**fence);
      commandPool->reset();
   }
}