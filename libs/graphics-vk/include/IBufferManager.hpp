#pragma once

#include "vk/ResourceManagerHandles.hpp"

namespace tr {

class Buffer;
class IGeometryData;

struct BufferCreateInfo {
  size_t size;
  vk::Flags<vk::BufferUsageFlagBits> flags;
  std::string name = "Unnamed Buffer";
  vma::MemoryUsage memoryUsage = vma::MemoryUsage::eCpuToGpu;
  vk::MemoryPropertyFlags memoryProperties = vk::MemoryPropertyFlagBits::eHostCoherent;
  bool mapped = false;
};

enum class ArenaBufferType : uint8_t {
  Vertex = 0,
  Index
};

struct ArenaBufferCreateInfo {
  size_t newItemStride;
  size_t initialBufferSize;
  ArenaBufferType bufferType;
  std::string_view bufferName;
};

struct ArenaGeometryBufferCreateInfo {
  size_t vertexSize;
  size_t indexSize;
  std::string_view bufferName;
};

class IBufferManager {
public:
  IBufferManager() = default;

  IBufferManager(const IBufferManager&) = default;
  IBufferManager(IBufferManager&&) = delete;
  auto operator=(const IBufferManager&) -> IBufferManager& = default;
  auto operator=(IBufferManager&&) -> IBufferManager& = delete;

  virtual ~IBufferManager() = default;

  virtual auto createBuffer(const BufferCreateInfo& bufferCreateInfo) -> BufferHandle = 0;

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

  virtual auto addToSingleBuffer(const void* data,
                                 size_t size,
                                 BufferHandle handle,
                                 vk::DeviceSize offset) -> void = 0;
  virtual auto removeData(BufferHandle handle, vk::DeviceSize offset, size_t size) -> void = 0;

  virtual auto createArenaBuffer(const ArenaBufferCreateInfo& createInfo) -> BufferHandle = 0;

  virtual auto createArenaGeometryBuffer(const ArenaGeometryBufferCreateInfo& createInfo)
      -> BufferHandle = 0;
};

}
