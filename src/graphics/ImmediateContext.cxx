#include "ImmediateContext.hpp"

namespace Triton::Graphics {

   ImmediateContext::ImmediateContext(const vk::raii::Device& device,
                                      const vk::raii::PhysicalDevice& physicalDevice,
                                      const vk::raii::Queue& newQueue,
                                      const uint32_t queueFamily,
                                      const std::string_view& name)
       : device(device), queue(newQueue) {

      // Create Command Pool
      const vk::CommandPoolCreateInfo transferCommandPoolCreateInfo{
          .flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
          .queueFamilyIndex = queueFamily,
      };

      commandPool = std::make_unique<vk::raii::CommandPool>(
          device.createCommandPool(transferCommandPoolCreateInfo));

      // Create Command Buffer
      const auto uploadAllocInfo =
          vk::CommandBufferAllocateInfo{.commandPool = **commandPool,
                                        .level = vk::CommandBufferLevel::ePrimary,
                                        .commandBufferCount = 1};

      commandBuffer = std::make_unique<vk::raii::CommandBuffer>(
          std::move(device.allocateCommandBuffers(uploadAllocInfo)[0]));

      tracyContext = TracyVkContext((*physicalDevice), (*device), *queue, *(*commandBuffer));

      TracyVkContextName(tracyContext, name.data(), name.length());

      // Create Fence
      fence = std::make_unique<vk::raii::Fence>(device.createFence(vk::FenceCreateInfo{}));
   }

   ImmediateContext::~ImmediateContext() {
      TracyVkDestroy(tracyContext);
   }

   void ImmediateContext::submit(std::function<void(vk::raii::CommandBuffer& cmd)>&& fn) const {
      constexpr vk::CommandBufferBeginInfo cmdBeginInfo{
          .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit};

      commandBuffer->begin(cmdBeginInfo);
      {
         TracyVkZone(tracyContext, *(*commandBuffer), "immediate submit");
         fn(*commandBuffer);
         TracyVkCollect(tracyContext, *(*commandBuffer));
      }
      commandBuffer->end();

      const auto submitInfo =
          vk::SubmitInfo{.commandBufferCount = 1, .pCommandBuffers = &(**commandBuffer)};

      queue.submit(submitInfo, **fence);

      // Eventually, this process needs barriers and stuff and should
      // just be waited for in the main rendering loop i think somehow further research is needed
      // In general, create an abstraction over loading textures that can be done
      // from an asnyc task
      if (const auto result = device.waitForFences(**fence, true, UINT64_MAX);
          result != vk::Result::eSuccess) {
         Log::warn << "During Immediate Submit, timeout waiting for fence" << std::endl;
      }
      device.resetFences(**fence);
      commandPool->reset();
   }
}