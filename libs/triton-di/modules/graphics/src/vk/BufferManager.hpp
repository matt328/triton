#pragma once

#include "cm/Rando.hpp"
#include "vk/ResourceManagerHandles.hpp"

namespace tr {

class Buffer;
class Allocator;
class ImmediateTransferContext;
class IGeometryData;

class BufferManager {
public:
  BufferManager(std::shared_ptr<Allocator> newAllocator,
                std::shared_ptr<ImmediateTransferContext> newImmediateTransferContext);
  ~BufferManager() = default;

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
      bool mapped = false) -> BufferHandle;

  auto createGpuVertexBuffer(size_t size, std::string_view name) -> BufferHandle;

  auto createGpuIndexBuffer(size_t size, std::string_view name) -> BufferHandle;

  auto createIndirectBuffer(size_t size) -> BufferHandle;

  [[nodiscard]] auto resizeBuffer(BufferHandle handle, size_t newSize) -> BufferHandle;

  [[nodiscard]] auto getBuffer(BufferHandle handle) const -> Buffer&;

  auto addToBuffer(const IGeometryData& geometryData,
                   BufferHandle vertexBufferHandle,
                   vk::DeviceSize vertexOffset,
                   BufferHandle indexBufferHandle,
                   vk::DeviceSize indexOffset) -> void;

private:
  std::shared_ptr<Allocator> allocator;
  std::shared_ptr<ImmediateTransferContext> immediateTransferContext;

  MapKey bufferMapKeygen;
  std::unordered_map<BufferHandle, std::unique_ptr<Buffer>> bufferMap;
};

}
