#pragma once
#include "CommandBufferManager.hpp"
#include <vk/VkResourceManager.hpp>

namespace tr::gfx {
   namespace task {
      class SyncManager;
   }

   class Frame {
    public:
      Frame(uint8_t newIndex,
            std::shared_ptr<CommandBufferManager> newCommandBufferManager,
            std::shared_ptr<VkResourceManager> newResourceManager,
            std::shared_ptr<queue::Graphics> newGraphicsQueue,
            std::shared_ptr<task::SyncManager> newSyncManager);

      auto beginFrame() -> void;
      auto endFrame() const -> void;

      [[nodiscard]] auto getIndex() const -> uint8_t;

    private:
      uint8_t index;
      std::shared_ptr<CommandBufferManager> commandBufferManager;
      std::shared_ptr<VkResourceManager> resourceManager;
      std::shared_ptr<queue::Graphics> graphicsQueue;
      std::shared_ptr<task::SyncManager> syncManager;

      std::string drawImageName;
      vk::raii::Fence fence;

      CommandBufferPtr staticCommandBuffer = CommandBufferManager::getEmpty();
      CommandBufferPtr startBuffer = CommandBufferManager::getEmpty();
      CommandBufferPtr endBuffer = CommandBufferManager::getEmpty();

      static auto transitionImage(const vk::raii::CommandBuffer& cmd,
                                  const vk::Image& image,
                                  vk::ImageLayout currentLayout,
                                  vk::ImageLayout newLayout) -> void;
   };

}
