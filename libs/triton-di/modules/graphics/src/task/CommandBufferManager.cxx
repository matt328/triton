#include "CommandBufferManager.hpp"

#include "SimpleObjectPool.hpp"

namespace tr::gfx {

   CommandBufferManager::CommandBufferManager(std::shared_ptr<Device> newDevice,
                                              const RendererConfig& rendererConfig)
       : device{std::move(newDevice)}, framesInFlight(rendererConfig.framesInFlight) {
   }
   CommandBufferManager::~CommandBufferManager() {
      Log.trace("Destroying CommandBufferManager");
   }

}
