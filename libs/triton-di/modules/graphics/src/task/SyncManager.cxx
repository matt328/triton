#include "SyncManager.hpp"

namespace tr::gfx::task {
   SyncManager::SyncManager(std::shared_ptr<Device> newDevice,
                            std::shared_ptr<CommandBufferManager> newCommandBufferManager)
       : device{std::move(newDevice)}, commandBufferManager{std::move(newCommandBufferManager)} {
   }

   SyncManager::~SyncManager() {
      Log.trace("Destroying sync manager");
   }

   auto SyncManager::acquireFence() -> vk::raii::Fence {
      if (!fencePool.empty()) {
         auto fence = std::move(fencePool.top());
         fencePool.pop();
         return fence;
      }

      return vk::raii::Fence(device->getVkDevice(), vk::FenceCreateInfo{});
   }

   auto SyncManager::recycleFence(vk::raii::Fence fence) -> void {
      device->getVkDevice().resetFences({*fence});
      fencePool.push(std::move(fence));
   }

   auto SyncManager::trackFence(vk::raii::Fence fence, std::vector<CommandBufferPtr> commandBuffers)
       -> void {
      pendingFences.push({std::move(fence), std::move(commandBuffers)});
   }

   auto SyncManager::poll() -> void {
      while (!pendingFences.empty()) {

         if (auto& pending = pendingFences.front();
             pending.fence.getStatus() == vk::Result::eSuccess) {
            pending.commandBuffers.clear(); // Clearing this should return the command buffer
            recycleFence(std::move(pending.fence));
            pendingFences.pop();
         } else {
            break;
         }
      }
   }
}
