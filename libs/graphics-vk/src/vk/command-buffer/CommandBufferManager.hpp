#pragma once

#include "vk/command-buffer/CommandBufferRequest.hpp"
#include "vk/command-buffer/PoolKey.hpp"

namespace tr {

class Device;
class IDebugManager;

using CommandBufferHandle = size_t;

struct CommandBufferUse {
  std::thread::id threadId;
  uint8_t frameId;
  std::string passId;
};

struct QueueConfig {
  QueueType queueType;
  std::vector<CommandBufferUse> uses;
};

struct CommandBufferInfo {
  std::vector<QueueConfig> queueConfigs;
};

class CommandBufferManager {
public:
  CommandBufferManager(std::shared_ptr<Device> newDevice,
                       std::shared_ptr<IDebugManager> newDebugManager);
  ~CommandBufferManager();

  CommandBufferManager(const CommandBufferManager&) = delete;
  CommandBufferManager(CommandBufferManager&&) = delete;
  auto operator=(const CommandBufferManager&) -> CommandBufferManager& = delete;
  auto operator=(CommandBufferManager&&) -> CommandBufferManager& = delete;

  auto allocateCommandBuffers(const CommandBufferInfo& info) -> void;
  auto requestCommandBuffer(const CommandBufferRequest& request) -> vk::raii::CommandBuffer&;

  /// Transfer CommandBuffers are more effective if used once and freed, so callers should let this
  /// go out of scope after it's been submitted, and request a fresh one each time.
  auto getTransferCommandBuffer() -> vk::raii::CommandBuffer;

private:
  std::shared_ptr<Device> device;
  std::shared_ptr<IDebugManager> debugManager;

  std::unordered_map<PoolKey, vk::raii::CommandPool> poolMap;
  std::unordered_map<CommandBufferRequest, vk::raii::CommandBuffer> bufferMap;

  std::unique_ptr<vk::raii::CommandPool> transferCommandPool;
};

}
