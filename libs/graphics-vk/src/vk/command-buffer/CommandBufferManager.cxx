#include "CommandBufferManager.hpp"

#include "vk/core/Device.hpp"
#include "gfx/IDebugManager.hpp"

namespace tr {
CommandBufferManager::CommandBufferManager(std::shared_ptr<Device> newDevice,
                                           std::shared_ptr<IDebugManager> newDebugManager)
    : device{std::move(newDevice)}, debugManager{std::move(newDebugManager)} {
  debugManager->setDevice(device);

  const auto transferCommandPoolCreateInfo = vk::CommandPoolCreateInfo{
      .flags = vk::CommandPoolCreateFlagBits::eTransient,
      .queueFamilyIndex = device->getTransferQueueFamily(),
  };

  transferCommandPool = std::make_unique<vk::raii::CommandPool>(
      device->getVkDevice().createCommandPool(transferCommandPoolCreateInfo));
  debugManager->setObjectName(**transferCommandPool, "CommandPool-Transfer");
}

CommandBufferManager::~CommandBufferManager() {
  bufferMap.clear();
  poolMap.clear();
}

auto CommandBufferManager::allocateCommandBuffers(const CommandBufferInfo& info) -> void {
  std::set<PoolKey> createdPools;

  for (const auto& queueConfig : info.queueConfigs) {
    const auto queueFamilyIndex = [&]() {
      switch (queueConfig.queueType) {
        case QueueType::Graphics:
          return device->getGraphicsQueueFamily();
        case QueueType::Compute:
          return device->getComputeQueueFamily();
        case QueueType::Transfer:
          return device->getTransferQueueFamily();
      }
      throw std::runtime_error("Unknown QueueType");
    }();

    for (const auto& use : queueConfig.uses) {
      const auto poolKey = PoolKey{.threadId = use.threadId,
                                   .frameId = use.frameId,
                                   .queueType = queueConfig.queueType};

      if (createdPools.insert(poolKey).second) {
        const auto poolCreateInfo = vk::CommandPoolCreateInfo{
            .flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
            .queueFamilyIndex = queueFamilyIndex,
        };
        poolMap.emplace(poolKey, device->getVkDevice().createCommandPool(poolCreateInfo));
      }
      assert(poolMap.contains(poolKey));
      const auto allocInfo = vk::CommandBufferAllocateInfo{
          .commandPool = *poolMap.at(poolKey),
          .level = vk::CommandBufferLevel::ePrimary,
          .commandBufferCount = 1,
      };

      const CommandBufferRequest req{
          .threadId = use.threadId,
          .frameId = use.frameId,
          .passId = use.passId,
          .queueType = queueConfig.queueType,
      };
      bufferMap.emplace(req,
                        std::move(device->getVkDevice().allocateCommandBuffers(allocInfo).front()));
    }
  }
}

auto CommandBufferManager::requestCommandBuffer(const CommandBufferRequest& request)
    -> vk::raii::CommandBuffer& {
  assert(bufferMap.contains(request) && "BufferMap does not contain requested command buffer");
  return bufferMap.at(request);
}

auto CommandBufferManager::getTransferCommandBuffer() -> vk::raii::CommandBuffer {
  const auto allocInfo = vk::CommandBufferAllocateInfo{.commandPool = *transferCommandPool,
                                                       .level = vk::CommandBufferLevel::ePrimary,
                                                       .commandBufferCount = 1};
  auto commandBuffers = device->getVkDevice().allocateCommandBuffers(allocInfo);
  return std::move(commandBuffers.front());
}
}
