#pragma once

#include "cm/Rando.hpp"

namespace tr {

class Device;
class IDebugManager;

using CommandBufferHandle = size_t;

class CommandBufferManager {
public:
  CommandBufferManager(std::shared_ptr<Device> newDevice,
                       std::shared_ptr<IDebugManager> newDebugManager);
  ~CommandBufferManager() = default;

  CommandBufferManager(const CommandBufferManager&) = delete;
  CommandBufferManager(CommandBufferManager&&) = delete;
  auto operator=(const CommandBufferManager&) -> CommandBufferManager& = delete;
  auto operator=(CommandBufferManager&&) -> CommandBufferManager& = delete;

  auto createGraphicsCommandBuffer() -> CommandBufferHandle;
  auto createTransferCommandBuffer() -> CommandBufferHandle;

  auto getCommandBuffer(CommandBufferHandle handle) -> vk::raii::CommandBuffer&;

private:
  std::shared_ptr<Device> device;
  std::shared_ptr<IDebugManager> debugManager;

  MapKey commandBufferMapKeygen;

  std::unique_ptr<vk::raii::CommandPool> graphicsCommandPool;
  std::unique_ptr<vk::raii::CommandPool> transferCommandPool;

  std::unordered_map<CommandBufferHandle, vk::raii::CommandBuffer> commandBufferMap;
  std::unordered_map<CommandBufferHandle, std::string> commandBufferNameMap;
};

}
