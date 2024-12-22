#include "CommandBufferManager.hpp"

#include "CommandBufferPool.hpp"
#include "gfx/QueueTypes.hpp"
#include "gfx/RenderContextConfig.hpp"

namespace tr {

   CommandBufferManager::CommandBufferManager(std::shared_ptr<Device> newDevice,
                                              std::shared_ptr<queue::Graphics> newGraphicsQueue,
                                              std::shared_ptr<queue::Transfer> newTransferQueue,
                                              std::shared_ptr<queue::Compute> newComputeQueue,
                                              const RenderContextConfig& rendererConfig)
       : device{std::move(newDevice)},
         graphicsQueue{std::move(newGraphicsQueue)},
         transferQueue{std::move(newTransferQueue)},
         computeQueue{std::move(newComputeQueue)},
         framesInFlight(rendererConfig.framesInFlight) {
      Log.trace("Creating command buffer manager, framesInFlight: {}", framesInFlight);
      registerType(PoolId::Transfer);
   }

   CommandBufferManager::~CommandBufferManager() {
      Log.trace("Destroying CommandBufferManager");
   }

   auto CommandBufferManager::registerType(const PoolId cmdType) -> void {
      if (cmdType == PoolId::Main) {
         for (uint32_t i = 0; i < framesInFlight; ++i) {
            {
               const auto key = getKey(i, cmdType, graphicsQueue->getFamily(), true);
               commandPools[key] =
                   std::make_unique<CommandBufferPool>(device, graphicsQueue->getFamily());
            }
            {
               const auto key = getKey(i, cmdType, graphicsQueue->getFamily(), false);
               commandPools[key] =
                   std::make_unique<CommandBufferPool>(device, graphicsQueue->getFamily());
            }
         }
      }
      if (cmdType == PoolId::Transfer) {
         const auto key = getKey(0, cmdType, transferQueue->getFamily(), true);
         commandPools[key] =
             std::make_unique<CommandBufferPool>(device, transferQueue->getFamily());
      }
   }

   auto CommandBufferManager::getPrimaryCommandBuffer(const uint8_t frameIndex,
                                                      const PoolId cmdType) -> CommandBufferPtr {
      assert(frameIndex < framesInFlight);
      uint32_t queueFamily = 0;
      if (cmdType == PoolId::Main) {
         queueFamily = graphicsQueue->getFamily();
      }
      if (const auto key = getKey(frameIndex, cmdType, queueFamily, true);
          commandPools.contains(key)) {
         return commandPools[key]->acquire();
      }
      throw std::out_of_range(
          std::format("Invalid frameIndex {} or queueFamily {} in getCommandBuffer",
                      frameIndex,
                      queueFamily));
   }

   auto CommandBufferManager::getSecondaryCommandBuffer(uint8_t frameIndex, PoolId cmdType)
       -> CommandBufferPtr {
      assert(frameIndex < framesInFlight);
      uint32_t queueFamily = 0;

      if (cmdType == PoolId::Main) {
         queueFamily = graphicsQueue->getFamily();
      }

      if (const auto key = getKey(frameIndex, cmdType, queueFamily, false);
          commandPools.contains(key)) {
         return commandPools[key]->acquire();
      }

      throw std::out_of_range(
          std::format("Invalid frameIndex {} or queueFamily {} in getCommandBuffer",
                      frameIndex,
                      queueFamily));
   }

   auto CommandBufferManager::getTransferCommandBuffer() -> CommandBufferPtr {
      if (const auto key = getKey(0, PoolId::Transfer, transferQueue->getFamily(), true);
          commandPools.contains(key)) {
         return commandPools[key]->acquire();
      }
      throw std::out_of_range(std::format("No transfer command buffer pool registered"));
   }

   auto CommandBufferManager::getEmpty() -> CommandBufferPtr {
      return CommandBufferPool::getEmpty();
   }

   auto CommandBufferManager::swapchainRecreated() -> void {
   }

   auto CommandBufferManager::getKey(const uint8_t frameIndex,
                                     PoolId cmdType,
                                     const uint32_t queueIndex,
                                     bool isPrimary) -> uint64_t {
      uint64_t key = 0;
      key |= static_cast<uint64_t>(frameIndex) << 56u; // Put byte1 in the highest byte
      key |= static_cast<uint64_t>(cmdType) << 48u;    // Put byte2 in the second-highest byte
      key |= static_cast<uint64_t>(queueIndex) << 1u;  // Place queueIndex in bits 1-31
      key |= static_cast<uint64_t>(isPrimary);         // Place isPrimary in lsb
      return key;
   }

}
