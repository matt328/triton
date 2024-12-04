#include "CommandBufferManager.hpp"

#include "CommandBufferPool.hpp"
#include "gfx/QueueTypes.hpp"

#include <gfx/IRenderContext.hpp>

namespace tr::gfx {

   CommandBufferManager::CommandBufferManager(std::shared_ptr<Device> newDevice,
                                              const std::shared_ptr<queue::Graphics>& graphicsQueue,
                                              const std::shared_ptr<queue::Transfer>& transferQueue,
                                              const std::shared_ptr<queue::Compute>& computeQueue,
                                              const RenderContextConfig& rendererConfig)
       : device{std::move(newDevice)}, framesInFlight(rendererConfig.framesInFlight) {
      Log.trace("Creating command buffer manager, framesInFlight: {}", framesInFlight);
      for (uint32_t i = 0; i < framesInFlight; ++i) {
         const auto key = getKey(i, graphicsQueue->getFamily());
         commandPools[key] =
             std::make_unique<CommandBufferPool>(device, graphicsQueue->getFamily());

         if (transferQueue->getFamily() != graphicsQueue->getFamily()) {
            const auto transferKey = getKey(i, transferQueue->getFamily());
            commandPools[transferKey] =
                std::make_unique<CommandBufferPool>(device, transferQueue->getFamily());
         }
         if (computeQueue->getFamily() != graphicsQueue->getFamily()) {
            const auto computeKey = getKey(i, computeQueue->getFamily());
            commandPools[computeKey] =
                std::make_unique<CommandBufferPool>(device, computeQueue->getFamily());
         }
      }
   }

   CommandBufferManager::~CommandBufferManager() {
      Log.trace("Destroying CommandBufferManager");
      commandPools.clear();
   }

   auto CommandBufferManager::getCommandBuffer(const uint32_t frameIndex,
                                               const uint32_t queueFamily) -> CommandBufferPtr {
      assert(frameIndex < framesInFlight);
      const auto key = getKey(frameIndex, queueFamily);
      return commandPools[key]->acquire();
   }
   auto CommandBufferManager::getKey(const uint32_t frameIndex, const uint32_t queueIndex)
       -> uint64_t {
      return (static_cast<uint64_t>(frameIndex) << 32) | queueIndex;
   }

}
