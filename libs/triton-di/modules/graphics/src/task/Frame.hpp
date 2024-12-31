#pragma once
#include "CommandBufferManager.hpp"
#include <vk/VkResourceManager.hpp>
#include <vulkan/vulkan_raii.hpp>

namespace tr {
namespace task {
class SyncManager;
}

enum class CmdBufferType : uint8_t {
  Main = 0,
  Start,
  End,
  Compute
};

class Frame {
public:
  explicit Frame(uint8_t newIndex,
                 vk::raii::Fence&& newRenderFence,
                 vk::raii::Semaphore&& newImageAvailableSemaphore,
                 vk::raii::Semaphore&& newRenderFinishedSemaphore,
                 vk::raii::Semaphore&& newComputeFinishedSemaphore);

  [[nodiscard]] auto getIndexedName(std::string_view input) const -> std::string;
  [[nodiscard]] auto getIndex() const -> uint8_t;
  [[nodiscard]] auto getImageAvailableSemaphore() -> vk::raii::Semaphore&;
  [[nodiscard]] auto getRenderFinishedSemaphore() -> vk::raii::Semaphore&;
  [[nodiscard]] auto getComputeFinishedSemaphore() -> vk::raii::Semaphore&;
  [[nodiscard]] auto getInFlightFence() -> vk::raii::Fence&;
  [[nodiscard]] auto getSwapchainImageIndex() const -> uint32_t;
  [[nodiscard]] auto getDrawImageId() const -> std::string;

  auto setSwapchainImageIndex(uint32_t index) -> void;
  auto setDrawImageExtent(vk::Extent2D extent) -> void;

  auto addCommandBuffer(CmdBufferType cmdType, CommandBufferPtr&& commandBuffer) -> void;
  [[nodiscard]] auto getCommandBuffer(CmdBufferType cmdType) const -> vk::raii::CommandBuffer&;
  auto clearCommandBuffers() -> void;

private:
  uint8_t index;

  std::string drawImageName;
  vk::raii::Fence inFlightFence;
  vk::raii::Semaphore imageAvailableSemaphore;
  vk::raii::Semaphore renderFinishedSemaphore;
  vk::raii::Semaphore computeFinishedSemaphore;

  uint32_t swapchainImageIndex{};
  vk::Extent2D drawImageExtent{};

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
