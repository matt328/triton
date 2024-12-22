#include "SyncManager.hpp"

namespace tr {
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

      return vk::raii::Fence(device->getVkDevice(),
                             vk::FenceCreateInfo{.flags = vk::FenceCreateFlagBits::eSignaled});
   }

   auto SyncManager::recycleFence(vk::raii::Fence fence) -> void {
      device->getVkDevice().resetFences({*fence});
      fencePool.push(std::move(fence));
   }

   auto SyncManager::awaitFence(const vk::raii::Fence& fence) const -> void {
      if (const auto res = device->getVkDevice().waitForFences(*fence, vk::True, UINT64_MAX);
          res != vk::Result::eSuccess) {
         throw std::runtime_error("Failed to wait fences");
      }
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
