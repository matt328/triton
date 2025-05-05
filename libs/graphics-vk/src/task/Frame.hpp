#pragma once
#include "bk/Handle.hpp"
#include "LogicalBufferHandle.hpp"
#include "img/ManagedImage.hpp"
#include "vk/command-buffer/CommandBufferManager.hpp"
#include "VkResourceManager.hpp"
#include "LogicalImageHandle.hpp"

namespace tr {

constexpr auto DepthImageName = "Image-Depth";

enum class CmdBufferType : uint8_t {
  Main = 0,
  Start,
  End,
  Compute
};

enum class BufferHandleType : uint8_t {
  StaticGpuBufferEntry = 0,
  StaticDrawCommand,
  StaticObjectDataBuffer,
  StaticObjectDataIndexBuffer,
  StaticCountBuffer,
  DynamicGpuBufferEntry,
  DynamicDrawCommand,
  DynamicObjectDataBuffer,
  DynamicObjectDataIndexBuffer,
  DynamicCountBuffer,
  CameraBuffer,
  DescriptorBuffer,
  AnimationDataBuffer,
  TerrainGpuBufferEntry,
  TerrainChunkDrawCommand,
  TerrainChunkData,
  TerrainChunkDataIndex,
  TerrainChunkCount,
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
  [[nodiscard]] auto getImageAvailableSemaphore() const -> const vk::raii::Semaphore&;
  [[nodiscard]] auto getRenderFinishedSemaphore() const -> const vk::raii::Semaphore&;
  [[nodiscard]] auto getComputeFinishedSemaphore() const -> const vk::raii::Semaphore&;
  [[nodiscard]] auto getInFlightFence() const -> const vk::raii::Fence&;
  [[nodiscard]] auto getSwapchainImageIndex() const noexcept -> uint32_t;
  [[nodiscard]] auto getRenderingInfo() const -> vk::RenderingInfo;
  [[nodiscard]] auto getDrawImageExtent() const -> vk::Extent2D;

  [[nodiscard]] auto getStaticObjectCount() const -> uint32_t;
  [[nodiscard]] auto getDynamicObjectCount() const -> uint32_t;
  [[nodiscard]] auto getTerrainChunkCount() const -> uint32_t;
  [[nodiscard]] auto getDebugObjectCount() const -> uint32_t;

  [[nodiscard]] auto getBufferHandle(BufferHandleType type) const -> BufferHandle;

  [[nodiscard]] auto getDepthImageHandle() const -> ImageHandle;
  [[nodiscard]] auto getDrawImageHandle() const -> ImageHandle;

  auto setBufferHandle(BufferHandleType type, BufferHandle handle) -> void;

  auto registerBuffer(BufferHandle handle) -> size_t;
  [[nodiscard]] auto getBufferHandle2(size_t key) const -> BufferHandle;

  auto addLogicalImage(Handle<ManagedImage> logicalHandle, Handle<ManagedImage> imageHandle)
      -> void;
  [[nodiscard]] auto getLogicalImage(Handle<ManagedImage> logicalHandle) const
      -> Handle<ManagedImage>;

  auto addLogicalBuffer(LogicalHandle<ManagedBuffer> logicalHandle,
                        Handle<ManagedBuffer> bufferHandle) -> void;

  [[nodiscard]] auto getLogicalBuffer(LogicalHandle<ManagedBuffer> logicalHandle) const
      -> Handle<ManagedBuffer>;

  auto setDepthImageHandle(ImageHandle handle) -> void;
  auto setDrawImageHandle(ImageHandle handle) -> void;

  auto setSwapchainImageIndex(uint32_t index) -> void;
  auto setDrawImageExtent(vk::Extent2D extent) -> void;

  auto setStaticObjectCount(uint32_t newObjectCount) -> void;
  auto setDynamicObjectCount(uint32_t newObjectCount) -> void;
  auto setTerrainChunkCount(uint32_t newObjectCount) -> void;
  auto setDebugObjectCount(uint32_t newDebugObjectCount) -> void;

  auto setupRenderingInfo(const std::shared_ptr<VkResourceManager>& resourceManager) -> void;

private:
  uint8_t index;

  vk::raii::Fence inFlightFence;
  vk::raii::Semaphore imageAvailableSemaphore;
  vk::raii::Semaphore renderFinishedSemaphore;
  vk::raii::Semaphore computeFinishedSemaphore;

  uint32_t swapchainImageIndex{};
  vk::Extent2D drawImageExtent{};

  std::unordered_map<LogicalHandle<ManagedBuffer>, Handle<ManagedBuffer>> bufferHandles;
  std::unordered_map<Handle<ManagedImage>, Handle<ManagedImage>> imageHandles;

  uint32_t staticObjectCount;
  uint32_t dynamicObjectCount;
  uint32_t terrainChunkCount;
  uint32_t debugObjectCount;

  static auto transitionImage(const vk::raii::CommandBuffer& cmd,
                              const vk::Image& image,
                              vk::ImageLayout currentLayout,
                              vk::ImageLayout newLayout) -> void;
};

}
