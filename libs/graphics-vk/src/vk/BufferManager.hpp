#pragma once

#include "IBufferManager.hpp"
#include "bk/HandleGenerator.hpp"
#include "bk/Rando.hpp"
#include "api/fx/IEventBus.hpp"
#include "vk/ResourceManagerHandles.hpp"
#include "vk/core/Device.hpp"

namespace tr {

class ManagedBuffer;
class Allocator;
class ImmediateTransferContext;
class IGeometryData;
class TaskQueue;
class BufferWrapper;

class BufferManager : public IBufferManager {
public:
  BufferManager(std::shared_ptr<Allocator> newAllocator,
                std::shared_ptr<ImmediateTransferContext> newImmediateTransferContext,
                std::shared_ptr<Device> newDevice,
                std::shared_ptr<TaskQueue> newTaskQueue,
                std::shared_ptr<IEventBus> eventBus);
  ~BufferManager() override = default;

  BufferManager(const BufferManager&) = delete;
  BufferManager(BufferManager&&) = delete;
  auto operator=(const BufferManager&) -> BufferManager& = delete;
  auto operator=(BufferManager&&) -> BufferManager& = delete;

  auto createBuffer(const BufferCreateInfo& bufferCreateInfo) -> BufferHandle override;

  auto createBuffer(
      size_t size,
      vk::Flags<vk::BufferUsageFlagBits> flags,
      std::string_view name,
      vma::MemoryUsage usage = vma::MemoryUsage::eCpuToGpu,
      vk::MemoryPropertyFlags memoryProperties = vk::MemoryPropertyFlagBits::eHostCoherent,
      bool mapped = false) -> BufferHandle override;

  auto createGpuVertexBuffer(size_t size, std::string_view name) -> BufferHandle override;

  auto createGpuIndexBuffer(size_t size, std::string_view name) -> BufferHandle override;

  auto createIndirectBuffer(size_t size) -> BufferHandle override;

  [[nodiscard]] auto resizeBuffer(BufferHandle handle, size_t newSize) -> BufferHandle override;

  [[nodiscard]] auto getBuffer(BufferHandle handle) const -> ManagedBuffer& override;

  auto addToSingleBuffer(const void* data, size_t size, BufferHandle handle, vk::DeviceSize offset)
      -> void override;

  auto removeData(BufferHandle handle, vk::DeviceSize offset, size_t size) -> void override;

  auto createStaticBuffer(const BufferCreateInfo& createInfo) -> Handle<BufferWrapper> override;

  auto createArenaBuffer(const ArenaBufferCreateInfo& createInfo) -> BufferHandle override;

  auto createArenaGeometryBuffer(const ArenaGeometryBufferCreateInfo& createInfo)
      -> BufferHandle override;

private:
  std::shared_ptr<Allocator> allocator;
  std::shared_ptr<ImmediateTransferContext> immediateTransferContext;
  std::shared_ptr<Device> device;
  std::shared_ptr<TaskQueue> taskQueue;

  HandleGenerator<BufferWrapper> bufferHandleGenerator;
  std::unordered_map<Handle<BufferWrapper>, std::unique_ptr<BufferWrapper>> newestBufferMap;

  MapKey bufferMapKeygen;
  std::unordered_map<BufferHandle, std::unique_ptr<ManagedBuffer>> bufferMap;
  std::unordered_map<BufferHandle, std::unique_ptr<BufferWrapper>> newBufferMap;
  std::vector<BufferHandle> unusedBuffers;
  bool clearInProgress = false;
};

}
