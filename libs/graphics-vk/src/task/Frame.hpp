#pragma once
#include "ImageTransitionQueue.hpp"
#include "api/gw/editordata/EditorState.hpp"
#include "bk/Handle.hpp"
#include "buffers/ManagedBuffer.hpp"
#include "img/ManagedImage.hpp"
#include "r3/graph/ImageAlias.hpp"
#include "r3/graph/ResourceAliases.hpp"
#include "r3/graph/barriers/LastBufferUse.hpp"
#include "r3/graph/barriers/LastImageUse.hpp"
#include "vk/ResourceManagerHandles.hpp"

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

class IShaderBinding;

class Frame {
public:
  explicit Frame(uint8_t newIndex,
                 vk::raii::Fence&& newRenderFence,
                 vk::raii::Semaphore&& newImageAvailableSemaphore,
                 vk::raii::Semaphore&& newComputeFinishedSemaphore);

  [[nodiscard]] auto getIndexedName(std::string_view input) const -> std::string;
  [[nodiscard]] auto getIndex() const -> uint8_t;
  [[nodiscard]] auto getImageAvailableSemaphore() const -> const vk::raii::Semaphore&;
  [[nodiscard]] auto getComputeFinishedSemaphore() const -> const vk::raii::Semaphore&;
  [[nodiscard]] auto getInFlightFence() const -> const vk::raii::Fence&;
  [[nodiscard]] auto getSwapchainImageIndex() const noexcept -> uint32_t;
  [[nodiscard]] auto getRenderingInfo() const -> vk::RenderingInfo;
  [[nodiscard]] auto getDrawImageExtent() const -> vk::Extent2D;

  [[nodiscard]] auto getObjectCount() const -> uint32_t;

  [[nodiscard]] auto getBufferHandle(BufferHandleType type) const -> BufferHandle;

  [[nodiscard]] auto getDepthImageHandle() const -> ImageHandle;
  [[nodiscard]] auto getDrawImageHandle() const -> ImageHandle;

  auto setBufferHandle(BufferHandleType type, BufferHandle handle) -> void;

  auto registerBuffer(BufferHandle handle) -> size_t;
  [[nodiscard]] auto getBufferHandle2(size_t key) const -> BufferHandle;

  auto addLogicalImage(LogicalHandle<ManagedImage> logicalHandle, Handle<ManagedImage> imageHandle)
      -> void;
  [[nodiscard]] auto getLogicalImage(LogicalHandle<ManagedImage> logicalHandle) const
      -> Handle<ManagedImage>;

  auto addLogicalBuffer(LogicalHandle<ManagedBuffer> logicalHandle,
                        Handle<ManagedBuffer> bufferHandle) -> void;
  auto addLogicalShaderBinding(LogicalHandle<IShaderBinding> logicalHandle,
                               Handle<IShaderBinding> handle) -> void;

  [[nodiscard]] auto getLogicalBuffer(LogicalHandle<ManagedBuffer> logicalHandle) const
      -> Handle<ManagedBuffer>;

  [[nodiscard]] auto getLogicalShaderBinding(LogicalHandle<IShaderBinding> logicalHandle) const
      -> Handle<IShaderBinding>;

  auto registerSwapchainLogicalHandle(LogicalHandle<ManagedImage> logicalHandle) -> void;
  auto addSwapchainImage(Handle<ManagedImage> handle, uint32_t index) -> void;

  auto setDepthImageHandle(ImageHandle handle) -> void;
  auto setDrawImageHandle(ImageHandle handle) -> void;

  auto setSwapchainImageIndex(uint32_t index) -> void;
  auto setDrawImageExtent(vk::Extent2D extent) -> void;

  auto setObjectCount(uint32_t newObjectCount) -> void;

  auto setLastImageUse(ImageAlias imageAlias, LastImageUse lastImageUse) -> void;
  auto getLastImageUse(ImageAlias imageAlias) const -> std::optional<LastImageUse>;

  auto setLastBufferUse(BufferAliasVariant bufferAlias, LastBufferUse lastBufferUse) -> void;
  auto getLastBufferUse(BufferAliasVariant bufferAlias) const -> std::optional<LastBufferUse>;

  auto setEditorState(std::optional<EditorState> newState) -> void;
  auto getEditorState() const -> std::optional<EditorState>;

  auto setImageTransitionInfo(const std::vector<ImageTransitionInfo>& transitionInfo) -> void;
  auto getImageTransitionInfo() -> std::vector<ImageTransitionInfo>;

private:
  uint8_t index;

  vk::raii::Fence inFlightFence;
  vk::raii::Semaphore imageAvailableSemaphore;
  vk::raii::Semaphore computeFinishedSemaphore;

  uint32_t swapchainImageIndex{};
  vk::Extent2D drawImageExtent{};

  std::unordered_map<LogicalHandle<ManagedBuffer>, Handle<ManagedBuffer>> bufferHandles;
  std::unordered_map<LogicalHandle<ManagedImage>, Handle<ManagedImage>> imageHandles;
  std::unordered_map<LogicalHandle<IShaderBinding>, Handle<IShaderBinding>> shaderBindingHandles;

  LogicalHandle<ManagedImage> swapchainLogicalHandle;
  std::unordered_map<uint32_t, Handle<ManagedImage>> swapchainImageHandles;

  std::unordered_map<ImageAlias, LastImageUse> lastImageUses;
  std::unordered_map<BufferAliasVariant, LastBufferUse> lastBufferUses;

  std::vector<ImageTransitionInfo> imageTransitionInfo;

  uint32_t objectCount;
  std::optional<EditorState> editorState;

  static auto transitionImage(const vk::raii::CommandBuffer& cmd,
                              const vk::Image& image,
                              vk::ImageLayout currentLayout,
                              vk::ImageLayout newLayout) -> void;
};

}
