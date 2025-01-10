#include "CommandBufferManager.hpp"

#include "vk/core/Device.hpp"
#include "tr/IDebugManager.hpp"

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
      .flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
      .queueFamilyIndex = device->getTransferQueueFamily(),
  };

  transferCommandPool = std::make_unique<vk::raii::CommandPool>(
      device->getVkDevice().createCommandPool(transferCommandPoolCreateInfo));
  debugManager->setObjectName(**transferCommandPool, "CommandPool-Transfer");
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

}
