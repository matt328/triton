#pragma once
#include "CommandBufferManager.hpp"
#include <vk/VkResourceManager.hpp>

namespace tr::gfx {
   namespace task {
      class SyncManager;
   }

   class Frame {
    public:
      explicit Frame(uint8_t newIndex,
                     vk::raii::Fence&& newRenderFence,
                     vk::raii::Semaphore&& newImageAvailableSemaphore,
                     vk::raii::Semaphore&& newRenderFinishedSemaphore);

      [[nodiscard]] auto getIndex() const -> uint8_t;
      [[nodiscard]] auto getStaticCommandBuffer() const -> vk::raii::CommandBuffer&;
      [[nodiscard]] auto getImageAvailableSemaphore() -> vk::raii::Semaphore&;
      [[nodiscard]] auto getInFlightFence() -> vk::raii::Fence&;
      [[nodiscard]] auto getSwapchainImageIndex() const -> uint32_t;

      auto setStaticCommandBuffer(CommandBufferPtr&& buffer) -> void;
      auto setSwapchainImageIndex(uint32_t index) -> void;

    private:
      uint8_t index;

      std::string drawImageName;
      vk::raii::Fence inFlightFence;
      vk::raii::Semaphore imageAvailableSemaphore;
      vk::raii::Semaphore renderFinishedSemaphore;

      uint32_t swapchainImageIndex;

      CommandBufferPtr staticCommandBuffer = CommandBufferManager::getEmpty();
      CommandBufferPtr startBuffer = CommandBufferManager::getEmpty();
      CommandBufferPtr endBuffer = CommandBufferManager::getEmpty();

      static auto transitionImage(const vk::raii::CommandBuffer& cmd,
                                  const vk::Image& image,
                                  vk::ImageLayout currentLayout,
                                  vk::ImageLayout newLayout) -> void;
   };

}
