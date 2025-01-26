#pragma once
#include "vk/CommandBufferManager.hpp"
#include "vk/VkResourceManager.hpp"

namespace tr {

constexpr auto DepthImageName = "Image-Depth";

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
                 vk::raii::Semaphore&& newComputeFinishedSemaphore,
                 CommandBufferHandle newStartCmdBuffer,
                 CommandBufferHandle newEndCmdBuffer,
                 CommandBufferHandle newMainCmdBuffer);

  [[nodiscard]] auto getIndexedName(std::string_view input) const -> std::string;
  [[nodiscard]] auto getIndex() const -> uint8_t;
  [[nodiscard]] auto getImageAvailableSemaphore() -> vk::raii::Semaphore&;
  [[nodiscard]] auto getRenderFinishedSemaphore() -> vk::raii::Semaphore&;
  [[nodiscard]] auto getComputeFinishedSemaphore() -> vk::raii::Semaphore&;
  [[nodiscard]] auto getInFlightFence() -> vk::raii::Fence&;
  [[nodiscard]] auto getSwapchainImageIndex() const noexcept -> uint32_t;
  [[nodiscard]] auto getRenderingInfo() const -> vk::RenderingInfo;
  [[nodiscard]] auto getDrawImageExtent() const -> vk::Extent2D;

  [[nodiscard]] auto getGpuBufferEntryBufferHandle() const -> BufferHandle;
  [[nodiscard]] auto getDrawCommandBufferHandle() const -> BufferHandle;
  [[nodiscard]] auto getGpuObjectDataBufferHandle() const -> BufferHandle;
  [[nodiscard]] auto getCameraBufferHandle() const -> BufferHandle;
  [[nodiscard]] auto getCountBufferHandle() const -> BufferHandle;
  [[nodiscard]] auto getDescriptorBufferHandle() const -> BufferHandle;
  [[nodiscard]] auto getObjectDataIndexBufferHandle() const -> BufferHandle;

  [[nodiscard]] auto getDepthImageHandle() const -> ImageHandle;
  [[nodiscard]] auto getDrawImageHandle() const -> ImageHandle;

  [[nodiscard]] auto getStartCommandBufferHandle() const -> CommandBufferHandle;
  [[nodiscard]] auto getMainCommandBufferHandle() const -> CommandBufferHandle;
  [[nodiscard]] auto getEndCommandBufferHandle() const -> CommandBufferHandle;

  auto setGpuBufferEntryBufferHandle(BufferHandle handle) -> void;
  auto setDrawCommandBufferHandle(BufferHandle handle) -> void;
  auto setGpuObjectDataBufferHandle(BufferHandle handle) -> void;
  auto setCameraBufferHandle(BufferHandle handle) -> void;
  auto setCountBufferHandle(BufferHandle handle) -> void;
  auto setDescriptorBufferHandle(BufferHandle handle) -> void;
  auto setObjectDataIndexBufferHandle(BufferHandle handle) -> void;

  auto setDepthImageHandle(ImageHandle handle) -> void;
  auto setDrawImageHandle(ImageHandle handle) -> void;

  auto setSwapchainImageIndex(uint32_t index) -> void;
  auto setDrawImageExtent(vk::Extent2D extent) -> void;
  auto setupRenderingInfo(const std::shared_ptr<VkResourceManager>& resourceManager) -> void;

private:
  uint8_t index;

  vk::raii::Fence inFlightFence;
  vk::raii::Semaphore imageAvailableSemaphore;
  vk::raii::Semaphore renderFinishedSemaphore;
  vk::raii::Semaphore computeFinishedSemaphore;

  uint32_t swapchainImageIndex{};
  vk::Extent2D drawImageExtent{};

  BufferHandle gpuBufferEntryBuffer;
  BufferHandle drawCommandBuffer;
  BufferHandle objectDataBuffer;
  BufferHandle cameraBuffer;
  BufferHandle countBuffer;
  BufferHandle objectDataIndexBuffer;
  BufferHandle descriptorBuffer;

  ImageHandle depthImageHandle;
  ImageHandle drawImageHandle;

  CommandBufferHandle startCmdBuffer;
  CommandBufferHandle mainCmdBuffer;
  CommandBufferHandle endCmdBuffer;

  vk::RenderingAttachmentInfo colorAttachmentInfo;
  vk::RenderingAttachmentInfo depthAttachmentInfo;
  vk::RenderingInfo renderingInfo;

  static auto transitionImage(const vk::raii::CommandBuffer& cmd,
                              const vk::Image& image,
                              vk::ImageLayout currentLayout,
                              vk::ImageLayout newLayout) -> void;
};

}
