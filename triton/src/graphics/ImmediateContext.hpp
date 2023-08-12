#pragma once

#include <vulkan/vulkan_raii.hpp>
namespace vk::raii {
   class Device;
   class CommandBuffer;
   class CommandPool;
   class Fence;
   class Queue;
}

class ImmediateContext {
 public:
   /**
    * Constructs an ImmediateContext with the given device, queue, and queueFamily.  Make
    * sure to match the queue and queueFamily params, apparently once you create a queue, you have
    * no idea which family it came from and we need that info to create a command pool for that
    * queue
    * \param device const ref to the Device's unique_ptr.
    * \param newQueue shared_ptr of the queue to use.
    * \param queueFamily family index matching the aforementioned queue.
    */
   ImmediateContext(const vk::raii::Device& device,
                    const vk::raii::PhysicalDevice& physicalDevice,
                    const vk::raii::Queue& newQueue,
                    uint32_t queueFamily,
                    const std::string_view& name = "unnamed immediate context");
   ~ImmediateContext();

   ImmediateContext(const ImmediateContext&) = delete;
   ImmediateContext(ImmediateContext&&) = delete;
   ImmediateContext& operator=(const ImmediateContext&) = delete;
   ImmediateContext& operator=(ImmediateContext&&) = delete;

   void submit(std::function<void(vk::raii::CommandBuffer& cmd)>&& fn) const;

 private:
   const vk::raii::Device& device; // NOLINT this is fine
   tracy::VkCtx* tracyContext;
   const vk::raii::Queue& queue; // NOLINT this is also file
   std::unique_ptr<vk::raii::Fence> fence = nullptr;
   std::unique_ptr<vk::raii::CommandPool> commandPool = nullptr;
   std::unique_ptr<vk::raii::CommandBuffer> commandBuffer = nullptr;

   // It's fine because RenderDevice will surely outlive ImmediateContext.  It's understood that
   // ImmediateContexts are not utility classes to be used wherever.
};
