#pragma once
#include "CommandBufferManager.hpp"
#include <vk/VkResourceManager.hpp>

namespace tr::gfx {
   namespace task {
      class SyncManager;
   }

   enum class CmdBufferType : uint8_t {
      Static = 0,
      Start,
      End
   };

   class Frame {
    public:
      explicit Frame(uint8_t newIndex,
                     vk::raii::Fence&& newRenderFence,
                     vk::raii::Semaphore&& newImageAvailableSemaphore,
                     vk::raii::Semaphore&& newRenderFinishedSemaphore);

      [[nodiscard]] auto getIndex() const -> uint8_t;
      [[nodiscard]] auto getImageAvailableSemaphore() -> vk::raii::Semaphore&;
      [[nodiscard]] auto getInFlightFence() -> vk::raii::Fence&;
      [[nodiscard]] auto getSwapchainImageIndex() const -> uint32_t;
      [[nodiscard]] auto getDrawImageId() const -> std::string;

      auto setSwapchainImageIndex(uint32_t index) -> void;

      auto addCommandBuffer(CmdBufferType cmdType, CommandBufferPtr&& commandBuffer) -> void;
      [[nodiscard]] auto getCommandBuffer(CmdBufferType cmdType) const -> vk::raii::CommandBuffer&;
      auto clearCommandBuffers() -> void;

    private:
      uint8_t index;

      std::string drawImageName;
      vk::raii::Fence inFlightFence;
      vk::raii::Semaphore imageAvailableSemaphore;
      vk::raii::Semaphore renderFinishedSemaphore;

      uint32_t swapchainImageIndex{};

      CommandBufferPtr staticCommandBuffer = CommandBufferManager::getEmpty();
      CommandBufferPtr startBuffer = CommandBufferManager::getEmpty();
      CommandBufferPtr endBuffer = CommandBufferManager::getEmpty();

      std::unordered_map<CmdBufferType, CommandBufferPtr> commandBuffers;

      static auto transitionImage(const vk::raii::CommandBuffer& cmd,
                                  const vk::Image& image,
                                  vk::ImageLayout currentLayout,
                                  vk::ImageLayout newLayout) -> void;
   };

}
