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
   // Constructs an immediate context which can be (re)used to execute command
   // buffers.
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

   // Executes the given function on the given command buffer in this context.
   void submit(std::function<void(vk::raii::CommandBuffer& cmd)>&& fn) const;

 private:
   const vk::raii::Device& device; // NOLINT this is fine
   tracy::VkCtx* tracyContext;
   const vk::raii::Queue& queue; // NOLINT this is also file
   std::unique_ptr<vk::raii::Fence> fence = nullptr;
   std::unique_ptr<vk::raii::CommandPool> commandPool = nullptr;
   std::unique_ptr<vk::raii::CommandBuffer> commandBuffer = nullptr;

   // It's fine because GraphicsContext will surely outlive ImmediateContext.  It's understood that
   // ImmediateContexts are not utility classes to be used wherever.
};
