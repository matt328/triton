#pragma once

#include "vk/ResourceManagerHandles.hpp"

namespace tr {

class Buffer;
class IGeometryData;

class IBufferManager {
public:
  IBufferManager() = default;

  IBufferManager(const IBufferManager&) = default;
  IBufferManager(IBufferManager&&) = delete;
  auto operator=(const IBufferManager&) -> IBufferManager& = default;
  auto operator=(IBufferManager&&) -> IBufferManager& = delete;

  virtual ~IBufferManager() = default;

  virtual auto createBuffer(size_t size,
                            vk::Flags<vk::BufferUsageFlagBits> flags,
                            std::string_view name,
                            vma::MemoryUsage usage,
                            vk::MemoryPropertyFlags memoryProperties,
                            bool mapped) -> BufferHandle = 0;

  virtual auto createGpuVertexBuffer(size_t size, std::string_view name) -> BufferHandle = 0;

  virtual auto createGpuIndexBuffer(size_t size, std::string_view name) -> BufferHandle = 0;

  virtual auto createIndirectBuffer(size_t size) -> BufferHandle = 0;

  [[nodiscard]] virtual auto resizeBuffer(BufferHandle handle, size_t newSize) -> BufferHandle = 0;

  [[nodiscard]] virtual auto getBuffer(BufferHandle handle) const -> Buffer& = 0;

  virtual auto addToBuffer(const IGeometryData& geometryData,
                           BufferHandle vertexBufferHandle,
                           vk::DeviceSize vertexOffset,
                           BufferHandle indexBufferHandle,
                           vk::DeviceSize indexOffset) -> void = 0;
};

}
