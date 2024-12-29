#pragma once
#include <vk/Device.hpp>

class CommandBufferPool {
public:
  CommandBufferPool(const std::shared_ptr<tr::Device>& device,
                    const uint32_t queueFamilyIndex,
                    const uint32_t initialCommandBufferCount = 6) {

    const auto commandPoolCreateInfo =
        vk::CommandPoolCreateInfo{.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
                                  .queueFamilyIndex = queueFamilyIndex};

    commandPool = std::make_unique<vk::raii::CommandPool>(
        device->getVkDevice().createCommandPool(commandPoolCreateInfo));

    const auto allocInfo =
        vk::CommandBufferAllocateInfo{.commandPool = **commandPool,
                                      .level = vk::CommandBufferLevel::ePrimary,
                                      .commandBufferCount = initialCommandBufferCount};

    for (auto& commandBuffer : device->getVkDevice().allocateCommandBuffers(allocInfo)) {
      add(std::make_unique<vk::raii::CommandBuffer>(std::move(commandBuffer)));
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

  [[nodiscard]] static auto getEmpty()
      -> std::unique_ptr<vk::raii::CommandBuffer, ReturnToPoolDeleter> {
    return {nullptr, ReturnToPoolDeleter{nullptr}};
  }

  [[nodiscard]] auto empty() const noexcept {
    return pool.empty();
  }

  [[nodiscard]] auto size() const noexcept {
    return pool.size();
  }

private:
  std::unique_ptr<vk::raii::CommandPool> commandPool;
  std::stack<std::unique_ptr<vk::raii::CommandBuffer>> pool;
};
