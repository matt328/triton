#pragma once

#include "bk/Handle.hpp"
#include "bk/HandleGenerator.hpp"
#include "buffers/BufferCreateInfo.hpp"
#include "buffers/ManagedBuffer.hpp"
#include "resources/allocators/IBufferAllocator.hpp"

namespace tr {

class IFrameManager;
struct BufferRequest;
struct BufferRegion;
class Device;
class Allocator;
class IBufferAllocator;
class FrameState;
class TransferSystem;

struct BufferEntry {
  BufferLifetime lifetime;
  std::deque<std::unique_ptr<ManagedBuffer>> versions;
  size_t currentSize{0};
};

class BufferSystem {
public:
  BufferSystem(std::shared_ptr<IFrameManager> newFrameManager,
               std::shared_ptr<Device> newDevice,
               std::shared_ptr<Allocator> newAllocator,
               std::shared_ptr<FrameState> newFramestate);
  ~BufferSystem();

  BufferSystem(const BufferSystem&) = delete;
  BufferSystem(BufferSystem&&) = delete;
  auto operator=(const BufferSystem&) -> BufferSystem& = delete;
  auto operator=(BufferSystem&&) -> BufferSystem& = delete;

  /// Registers a single buffer based on the BufferRequest, and returns a Handle to the buffer which
  /// can be used in subsequent method calls to work with the created buffer.
  auto registerBuffer(const BufferCreateInfo& createInfo) -> Handle<ManagedBuffer>;

  /// Registers (framesCount) buffers with each frame by consulting the IFrameManager. Takes in A
  /// BufferRequest that describes the buffer and returns a LogicalHandle, which must be exchanged
  /// for a Handle with a Frame.
  auto registerPerFrameBuffer(const BufferCreateInfo& createInfo) -> LogicalHandle<ManagedBuffer>;

  /// Write `size` data at the specified `offset` in the buffer.
  auto insert(Handle<ManagedBuffer> handle, void* data, const BufferRegion& targetRegion)
      -> std::optional<BufferRegion>;

  /// Removes the data from the buffer described by the given `BufferRegion`. Should be quick as it
  /// doesn't change the buffer on the GPU, but just returns the given region to the free list on
  /// the CPU side.
  auto removeData(Handle<ManagedBuffer> handle, const BufferRegion& region) -> void;

  /// Attempts to reserve a BufferRegion of `size` in the buffer given by `handle`. On success, the
  /// `AllocationResult` will contain a `BufferRegion`. If the buffer must be resized first, the
  /// `AllocationResult` will contain a `ResizeRequest`. Allocation and resizing are separated out
  /// from `BufferSystem::insert` since clients may want to batch buffer resize operations.
  auto allocate(Handle<ManagedBuffer> handle, size_t size) -> BufferRegion;

  auto checkSize(Handle<ManagedBuffer> handle, size_t size) -> std::optional<ResizeRequest>;

  /// Gets the buffer's address as a uint64_t to be set into a PushConstant.
  auto getBufferAddress(Handle<ManagedBuffer> handle) -> std::optional<uint64_t>;

  /// Escape Hatch to get the `vk::Buffer`
  auto getVkBuffer(Handle<ManagedBuffer> handle) -> std::optional<const vk::Buffer*>;

  /// Resizes the given buffer by allocating a new buffer of `newSize` and copying the existing
  /// buffer contents into it. Returns a fence that will be signaled when the operation is
  /// completed. BufferSystem will handle replacing the buffer transparently to the renderer, always
  /// presenting a consistent buffer to he renderer and ensuring the frames in flight are flushed
  /// before deleting the old buffer.
  auto resize(const std::shared_ptr<TransferSystem>& transferSystem,
              const std::vector<ResizeRequest>& resizeRequests) -> void;

  auto pruneOldVersions() -> void;

private:
  std::shared_ptr<IFrameManager> frameManager;
  std::shared_ptr<Device> device;
  std::shared_ptr<Allocator> allocator;
  std::shared_ptr<FrameState> frameState;

  HandleGenerator<ManagedBuffer> bufferHandleGenerator;
  std::unordered_map<Handle<ManagedBuffer>, std::unique_ptr<BufferEntry>> bufferMap;
  std::unordered_map<Handle<ManagedBuffer>, std::unique_ptr<IBufferAllocator>> allocatorMap;

  [[nodiscard]] auto getCurrentManagedBuffer(Handle<ManagedBuffer> handle)
      -> std::optional<ManagedBuffer*>;

  [[nodiscard]] auto getCurrentManagedBufferConst(Handle<ManagedBuffer> handle) const
      -> std::optional<const ManagedBuffer*>;

  static auto fromCreateInfo(const BufferCreateInfo& createInfo)
      -> std::tuple<vk::BufferCreateInfo, vma::AllocationCreateInfo>;
};

}
