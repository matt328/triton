#include "CommandBufferManager.hpp"

#include "CommandBufferPool.hpp"
#include "gfx/QueueTypes.hpp"
#include "gfx/RenderContextConfig.hpp"

namespace tr::gfx {

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
   }

   CommandBufferManager::~CommandBufferManager() {
      Log.trace("Destroying CommandBufferManager");
   }

   auto CommandBufferManager::registerType(const CommandBufferType cmdType) {
      if (cmdType == CommandBufferType::StaticTasks) {
         for (uint32_t i = 0; i < framesInFlight; ++i) {
            const auto key = getKey(i, cmdType, graphicsQueue->getFamily());
            commandPools[key] =
                std::make_unique<CommandBufferPool>(device, graphicsQueue->getFamily());
         }
      }
   }

   auto CommandBufferManager::getCommandBuffer(const uint8_t frameIndex,
                                               const CommandBufferType cmdType)
       -> CommandBufferPtr {
      assert(frameIndex < framesInFlight);
      uint32_t queueFamily = 0;
      if (cmdType == CommandBufferType::StaticTasks) {
         queueFamily = graphicsQueue->getFamily();
      }
      if (const auto key = getKey(frameIndex, cmdType, queueFamily); commandPools.contains(key)) {
         return commandPools[key]->acquire();
      }
      throw std::out_of_range(
          std::format("Invalid frameIndex {} or queueFamily {} in getCommandBuffer",
                      frameIndex,
                      queueFamily));
   }
   auto CommandBufferManager::getEmpty() -> CommandBufferPtr {
      return CommandBufferPool::getEmpty();
   }

   auto CommandBufferManager::getKey(const uint8_t frameIndex,
                                     CommandBufferType cmdType,
                                     const uint32_t queueIndex) -> uint64_t {
      uint64_t key = 0;
      key |= static_cast<uint64_t>(frameIndex) << 56u; // Put byte1 in the highest byte
      key |= static_cast<uint64_t>(cmdType) << 48u;    // Put byte2 in the second-highest byte
      key |= static_cast<uint64_t>(queueIndex);        // Place value in the lower 4 bytes
      return key;
   }

}
