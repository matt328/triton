#pragma once
#include <vk/Device.hpp>

struct ReturnToPoolDeleter;

using CommandBufferPtr = std::unique_ptr<vk::raii::CommandBuffer, ReturnToPoolDeleter>;

class CommandBufferPool {
 public:
   CommandBufferPool(const std::shared_ptr<tr::gfx::Device>& device,
                     const uint32_t queueFamilyIndex,
                     const uint32_t initialCommandBufferCount = 3) {
      Log.trace("Creating CommandBufferPool: queueFamily: {}, initialCommandBufferCount: {}",
                queueFamilyIndex,
                initialCommandBufferCount);
      const auto commandPoolCreateInfo =
          vk::CommandPoolCreateInfo{.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
                                    .queueFamilyIndex = queueFamilyIndex};

      commandPool = std::make_unique<vk::raii::CommandPool>(
          device->getVkDevice().createCommandPool(commandPoolCreateInfo));

      const auto allocInfo =
          vk::CommandBufferAllocateInfo{.commandPool = **commandPool,
                                        .level = vk::CommandBufferLevel::ePrimary,
                                        .commandBufferCount = initialCommandBufferCount};
      auto commandBuffers = device->getVkDevice().allocateCommandBuffers(allocInfo);

      for (size_t i = 0; i < commandBuffers.size(); ++i) {
         add(std::make_unique<vk::raii::CommandBuffer>(std::move(commandBuffers[i])));
      }
   }

   CommandBufferPool(const CommandBufferPool&) = delete;
   CommandBufferPool(CommandBufferPool&&) = delete;
   auto operator=(const CommandBufferPool&) -> CommandBufferPool& = delete;
   auto operator=(CommandBufferPool&&) -> CommandBufferPool& = delete;

   ~CommandBufferPool() {
      Log.trace("Destroying CommandBufferPool");
   }

   void add(std::unique_ptr<vk::raii::CommandBuffer> obj) {
      pool.push(std::move(obj));
   }
   struct ReturnToPoolDeleter {
      explicit ReturnToPoolDeleter(CommandBufferPool* pool) : pool(pool) {
      }

      void operator()(vk::raii::CommandBuffer* ptr) const {
         pool->add(std::unique_ptr<vk::raii::CommandBuffer>{ptr});
      }

    private:
      CommandBufferPool* pool;
   };

   auto acquire() -> std::unique_ptr<vk::raii::CommandBuffer, ReturnToPoolDeleter> {
      if (pool.empty()) {
         throw std::out_of_range("Cannot acquire object from an empty pool.");
      }

      auto obj = std::move(pool.top());
      pool.pop();

      return {obj.release(), ReturnToPoolDeleter{this}};
   }

   [[nodiscard]] auto empty() const -> bool {
      return pool.empty();
   }

   [[nodiscard]] auto size() const -> size_t {
      return pool.size();
   }

 private:
   std::stack<std::unique_ptr<vk::raii::CommandBuffer>> pool;
   std::unique_ptr<vk::raii::CommandPool> commandPool;
};
