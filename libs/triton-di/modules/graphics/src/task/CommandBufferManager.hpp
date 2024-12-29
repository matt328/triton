#pragma once
#include "CommandBufferPool.hpp"

#include "vk/Device.hpp"
#include "PoolId.hpp"
#include "gfx/RenderContextConfig.hpp"

namespace tr {
namespace queue {
class Compute;
class Transfer;
class Graphics;
}

using CommandBufferPtr =
    std::unique_ptr<vk::raii::CommandBuffer, CommandBufferPool::ReturnToPoolDeleter>;

class CommandBufferManager {
public:
  CommandBufferManager(std::shared_ptr<Device> newDevice,
                       std::shared_ptr<queue::Graphics> newGraphicsQueue,
                       std::shared_ptr<queue::Transfer> newTransferQueue,
                       std::shared_ptr<queue::Compute> newComputeQueue,
                       const RenderContextConfig& rendererConfig);
  ~CommandBufferManager();

  CommandBufferManager(const CommandBufferManager&) = delete;
  CommandBufferManager(CommandBufferManager&&) = delete;
  auto operator=(const CommandBufferManager&) -> CommandBufferManager& = delete;
  auto operator=(CommandBufferManager&&) -> CommandBufferManager& = delete;

  auto registerType(PoolId cmdType) -> void;

  auto getPrimaryCommandBuffer(uint8_t frameIndex, PoolId cmdType) -> CommandBufferPtr;
  auto getSecondaryCommandBuffer(uint8_t frameIndex, PoolId cmdType) -> CommandBufferPtr;
  auto getTransferCommandBuffer() -> CommandBufferPtr;
  static auto getEmpty() -> CommandBufferPtr;

  auto swapchainRecreated() -> void;

private:
  std::shared_ptr<Device> device;
  std::shared_ptr<queue::Graphics> graphicsQueue;
  std::shared_ptr<queue::Transfer> transferQueue;
  std::shared_ptr<queue::Compute> computeQueue;

  std::shared_ptr<CommandBufferPool> commandBufferPool;
  uint8_t framesInFlight;

  std::unordered_map<uint64_t, std::unique_ptr<CommandBufferPool>> commandPools;

  static auto getKey(uint8_t frameIndex, PoolId cmdType, uint32_t queueIndex, bool isPrimary)
      -> uint64_t;
};

}
