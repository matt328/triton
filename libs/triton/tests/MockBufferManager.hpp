#pragma once

#include "IBufferManager.hpp"

namespace tr {

class Buffer;

class MockBufferManager : public tr::IBufferManager {
  auto createBuffer([[maybe_unused]] size_t size,
                    [[maybe_unused]] vk::Flags<vk::BufferUsageFlagBits> flags,
                    [[maybe_unused]] std::string_view name,
                    [[maybe_unused]] vma::MemoryUsage usage,
                    [[maybe_unused]] vk::MemoryPropertyFlags memoryProperties,
                    [[maybe_unused]] bool mapped) -> BufferHandle override {
    return 0;
  }

  auto createGpuVertexBuffer([[maybe_unused]] size_t size, [[maybe_unused]] std::string_view name)
      -> BufferHandle override {
    return 0;
  }

  auto createGpuIndexBuffer([[maybe_unused]] size_t size, [[maybe_unused]] std::string_view name)
      -> BufferHandle override {
    return 0;
  }

  auto createIndirectBuffer([[maybe_unused]] size_t size) -> BufferHandle override {
    return 0;
  }

  [[nodiscard]] auto resizeBuffer([[maybe_unused]] BufferHandle handle,
                                  [[maybe_unused]] size_t newSize) -> BufferHandle override {
    return 0;
  }

  [[nodiscard]] auto getBuffer([[maybe_unused]] BufferHandle handle) const -> Buffer& override {
  }

  auto addToBuffer([[maybe_unused]] const IGeometryData& geometryData,
                   [[maybe_unused]] BufferHandle vertexBufferHandle,
                   [[maybe_unused]] vk::DeviceSize vertexOffset,
                   [[maybe_unused]] BufferHandle indexBufferHandle,
                   [[maybe_unused]] vk::DeviceSize indexOffset) -> void override {
  }
};
}
