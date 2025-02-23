#pragma once

#include "IBufferManager.hpp"
#include "cm/Rando.hpp"
#include "tr/IEventBus.hpp"
#include "vk/ResourceManagerHandles.hpp"
#include "vk/core/Device.hpp"

namespace tr {

class Buffer;
class Allocator;
class ImmediateTransferContext;
class IGeometryData;
class TaskQueue;

class BufferManager : public IBufferManager {
public:
  BufferManager(std::shared_ptr<Allocator> newAllocator,
                std::shared_ptr<ImmediateTransferContext> newImmediateTransferContext,
                std::shared_ptr<Device> newDevice,
                std::shared_ptr<TaskQueue> newTaskQueue,
                const std::shared_ptr<IEventBus>& eventBus);
  ~BufferManager() override = default;

  BufferManager(const BufferManager&) = delete;
  BufferManager(BufferManager&&) = delete;
  auto operator=(const BufferManager&) -> BufferManager& = delete;
  auto operator=(BufferManager&&) -> BufferManager& = delete;

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

  [[nodiscard]] auto getBuffer(BufferHandle handle) const -> Buffer& override;

  auto addToBuffer(const IGeometryData& geometryData,
                   BufferHandle vertexBufferHandle,
                   vk::DeviceSize vertexOffset,
                   BufferHandle indexBufferHandle,
                   vk::DeviceSize indexOffset) -> void override;

  auto addToSingleBuffer(const void* data,
                         size_t size,
                         BufferHandle handle,
                         vk::DeviceSize offset) -> void override;

  auto removeData(BufferHandle handle, vk::DeviceSize offset, size_t size) -> void override;

private:
  std::shared_ptr<Allocator> allocator;
  std::shared_ptr<ImmediateTransferContext> immediateTransferContext;
  std::shared_ptr<Device> device;
  std::shared_ptr<TaskQueue> taskQueue;

  MapKey bufferMapKeygen;
  std::unordered_map<BufferHandle, std::unique_ptr<Buffer>> bufferMap;
  std::vector<BufferHandle> unusedBuffers;
  bool clearInProgress = false;
};

}
