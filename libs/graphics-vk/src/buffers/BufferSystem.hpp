#pragma once

#include "bk/Handle.hpp"
#include "bk/HandleGenerator.hpp"
#include "buffers/IBufferStrategy.hpp"
#include "buffers/ManagedBuffer.hpp"

namespace tr {

class IFrameManager;
struct BufferRequest;
struct BufferRegion;
class Device;
class Allocator;
struct BufferCreateInfo;

class BufferSystem {
public:
  BufferSystem(std::shared_ptr<IFrameManager> newFrameManager,
               std::shared_ptr<Device> newDevice,
               std::shared_ptr<Allocator> newAllocator);
  ~BufferSystem() = default;

  BufferSystem(const BufferSystem&) = delete;
  BufferSystem(BufferSystem&&) = delete;
  auto operator=(const BufferSystem&) -> BufferSystem& = delete;
  auto operator=(BufferSystem&&) -> BufferSystem& = delete;

  /// Registers a single buffer based on the BufferRequest, and returns a Handle to the buffer which
  /// can be used in subsequent method calls to work with the created buffer.
  auto registerBuffer(BufferCreateInfo createInfo) -> Handle<ManagedBuffer>;

  /// Registers (framesCount) buffers with each frame by consulting the IFrameManager. Takes in A
  /// BufferRequest that describes the buffer and returns a LogicalHandle, which must be exchanged
  /// for a Handle with a Frame.
  auto registerPerFrameBuffer(BufferCreateInfo createInfo) -> LogicalHandle<ManagedBuffer>;

  /// Write `size` data to the beginning to the beginning of the buffer.
  auto rewrite(Handle<ManagedBuffer> handle, const void* data, size_t size) -> void;

  auto insert(Handle<ManagedBuffer> handle, const void* data, size_t size) -> BufferRegion;

  /// Removes the data from the buffer described by the given `BufferRegion`. Should be quick as it
  /// doesn't change the buffer on the GPU, but just returns the given region to the free list on
  /// the CPU side.
  auto removeData(Handle<ManagedBuffer> handle, const BufferRegion& region) -> void;

  /// Gets the buffer's address as a uint64_t to be set into a PushConstant.
  auto getBufferAddress(Handle<ManagedBuffer> handle) -> uint64_t;

private:
  std::shared_ptr<IFrameManager> frameManager;
  std::shared_ptr<Device> device;
  std::shared_ptr<Allocator> allocator;

  HandleGenerator<ManagedBuffer> bufferHandleGenerator;
  std::unordered_map<Handle<ManagedBuffer>, ManagedBuffer> bufferMap;

  HandleGenerator<IBufferStrategy> strategyHandleGenerator;
  std::unordered_map<Handle<IBufferStrategy>, std::unique_ptr<IBufferStrategy>> strategyMap;

  static auto fromCreateInfo(const BufferCreateInfo& createInfo)
      -> std::tuple<vk::BufferCreateInfo, vma::AllocationCreateInfo>;
};

}
