#pragma once
#include "CommandBufferManager.hpp"
#include "vk/Device.hpp"

namespace tr::gfx::task {

   class SyncManager {
    public:
      SyncManager(std::shared_ptr<Device> newDevice,
                  std::shared_ptr<CommandBufferManager> newCommandBufferManager);
      ~SyncManager();

      SyncManager(const SyncManager&) = delete;
      SyncManager(SyncManager&&) = delete;
      auto operator=(const SyncManager&) -> SyncManager& = delete;
      auto operator=(SyncManager&&) -> SyncManager& = delete;

      auto acquireFence() -> vk::raii::Fence;
      auto recycleFence(vk::raii::Fence fence) -> void;

      auto awaitFence(const vk::raii::Fence& fence) const -> void;

      auto trackFence(vk::raii::Fence fence, std::vector<CommandBufferPtr> commandBuffers) -> void;
      auto poll() -> void;

    private:
      std::shared_ptr<Device> device;
      std::shared_ptr<CommandBufferManager> commandBufferManager;

      std::stack<vk::raii::Fence> fencePool;

      struct PendingFence {
         vk::raii::Fence fence;
         std::vector<CommandBufferPtr> commandBuffers;
      };

      std::queue<PendingFence> pendingFences;
   };

}
