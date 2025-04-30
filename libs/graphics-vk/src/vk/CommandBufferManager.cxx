#include "CommandBufferManager.hpp"

#include "vk/core/Device.hpp"
#include "gfx/IDebugManager.hpp"

namespace tr {
CommandBufferManager::CommandBufferManager(std::shared_ptr<Device> newDevice,
                                           std::shared_ptr<IDebugManager> newDebugManager)
    : device{std::move(newDevice)}, debugManager{std::move(newDebugManager)} {
  debugManager->setDevice(device);

  const auto commandPoolCreateInfo = vk::CommandPoolCreateInfo{
      .flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
      .queueFamilyIndex = device->getGraphicsQueueFamily(),
  };

  graphicsCommandPool = std::make_unique<vk::raii::CommandPool>(
      device->getVkDevice().createCommandPool(commandPoolCreateInfo));
  debugManager->setObjectName(**graphicsCommandPool, "CommandPool-Graphics");

  const auto transferCommandPoolCreateInfo = vk::CommandPoolCreateInfo{
      .flags = vk::CommandPoolCreateFlagBits::eTransient,
      .queueFamilyIndex = device->getTransferQueueFamily(),
  };

  transferCommandPool = std::make_unique<vk::raii::CommandPool>(
      device->getVkDevice().createCommandPool(transferCommandPoolCreateInfo));
  debugManager->setObjectName(**transferCommandPool, "CommandPool-Transfer");
}

auto CommandBufferManager::allocateCommandBuffers(const CommandBufferInfo& info) -> void {
  for (const auto& queueConfig : info.queueConfigs) {
    // Create a pool for each thread and frame
    // allocate command buffers for each passId, map to passIds
  }
}

auto CommandBufferManager::requestCommandBuffer(const CommandBufferRequest& request)
    -> vk::raii::CommandBuffer& {
  return bufferMap.at(request);
}

auto CommandBufferManager::createGraphicsCommandBuffer() -> CommandBufferHandle {
  const auto key = commandBufferMapKeygen.getKey();

  const auto allocInfo = vk::CommandBufferAllocateInfo{.commandPool = *graphicsCommandPool,
                                                       .level = vk::CommandBufferLevel::ePrimary,
                                                       .commandBufferCount = 1};
  auto commandBuffers = device->getVkDevice().allocateCommandBuffers(allocInfo);
  commandBufferMap.emplace(key, std::move(commandBuffers.front()));

  return key;
}

auto CommandBufferManager::createTransferCommandBuffer() -> CommandBufferHandle {
  const auto key = commandBufferMapKeygen.getKey();

  const auto allocInfo = vk::CommandBufferAllocateInfo{.commandPool = *transferCommandPool,
                                                       .level = vk::CommandBufferLevel::ePrimary,
                                                       .commandBufferCount = 1};
  auto commandBuffers = device->getVkDevice().allocateCommandBuffers(allocInfo);

  commandBufferMap.emplace(key, std::move(commandBuffers.front()));

  return key;
}

[[nodiscard]] auto CommandBufferManager::getCommandBuffer(CommandBufferHandle handle)
    -> vk::raii::CommandBuffer& {
  return commandBufferMap.at(handle);
}

auto CommandBufferManager::getTransferCommandBuffer() -> vk::raii::CommandBuffer {
  const auto allocInfo = vk::CommandBufferAllocateInfo{.commandPool = *transferCommandPool,
                                                       .level = vk::CommandBufferLevel::ePrimary,
                                                       .commandBufferCount = 1};
  auto commandBuffers = device->getVkDevice().allocateCommandBuffers(allocInfo);
  return std::move(commandBuffers.front());
}

}
