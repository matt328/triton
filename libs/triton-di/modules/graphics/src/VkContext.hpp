#pragma once

namespace vk::raii {
   class Device;
   class CommandBuffer;
   class CommandPool;
   class Fence;
   class Queue;
}

namespace tr::gfx {

   class VkContext {
    public:
      // Constructs an immediate context which can be (re)used to execute command
      // buffers.
      VkContext(const vk::raii::Device& device,
                const vk::raii::PhysicalDevice& physicalDevice,
                uint32_t queueIndex,
                uint32_t queueFamily,
                const std::string_view& name = "unnamed immediate context");
      ~VkContext();

      VkContext(const VkContext&) = delete;
      VkContext(VkContext&&) = delete;
      auto operator=(const VkContext&) -> VkContext& = delete;
      auto operator=(VkContext&&) -> VkContext& = delete;

      // Executes the given function on the given command buffer in this context.
      void submit(std::function<void(vk::raii::CommandBuffer& cmd)>&& fn) const;

    private:
      const vk::raii::Device& device; // NOLINT this is fine
      const vk::raii::PhysicalDevice& physicalDevice;
      tracy::VkCtx* tracyContext;
      std::unique_ptr<vk::raii::Queue> queue = nullptr;
      std::unique_ptr<vk::raii::Fence> fence = nullptr;
      std::unique_ptr<vk::raii::CommandPool> commandPool = nullptr;
      std::unique_ptr<vk::raii::CommandBuffer> commandBuffer = nullptr;

      // It's fine because GraphicsContext will surely outlive VkContext.  It's understood
      // that ImmediateContexts are not utility classes to be used wherever.
   };
}