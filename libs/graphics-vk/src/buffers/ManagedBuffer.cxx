#include "ManagedBuffer.hpp"
#include "api/gfx/GpuMaterialData.hpp"

namespace tr {

ManagedBuffer::ManagedBuffer(vk::Buffer newVkBuffer,
                             BufferMeta newBufferMeta,
                             std::shared_ptr<vma::Allocator> newAllocator,
                             vma::Allocation newAllocation)
    : vkBuffer{newVkBuffer},
      bufferMeta{std::move(newBufferMeta)},
      allocation{newAllocation},
      allocator{std::move(newAllocator)} {
}

ManagedBuffer::~ManagedBuffer() {
  if (vkBuffer && allocation) {
    if (this->mappedData != nullptr) {
      allocator->unmapMemory(allocation);
    }
    allocator->destroyBuffer(vkBuffer, allocation);
  }
}

auto ManagedBuffer::isMappable() -> bool {
  const auto memProps = allocator->getAllocationMemoryProperties(allocation);
  return (memProps & vk::MemoryPropertyFlagBits::eHostCoherent) != vk::MemoryPropertyFlagBits{};
}

auto ManagedBuffer::map() -> void {
  if (this->mappedData == nullptr) {
    try {
      this->mappedData = allocator->mapMemory(allocation);
    } catch (const std::runtime_error& e) {
      Log.error("Failed to map memory: {}", e.what());
      this->mappedData = nullptr;
    }
  }
}

auto ManagedBuffer::uploadData(const void* srcData, size_t size, size_t offset) -> void {
  if (this->mappedData == nullptr) {
    map();
  }
  // This is segfaulting for some reason
  std::memcpy(static_cast<char*>(this->mappedData) + offset, srcData, size);
}

[[nodiscard]] auto ManagedBuffer::getVkBuffer() const -> const vk::Buffer& {
  return vkBuffer;
}

[[nodiscard]] auto ManagedBuffer::getMeta() const -> const BufferMeta& {
  return bufferMeta;
}

auto ManagedBuffer::getValidFromFrame() const -> uint64_t {
  return validFromFrame;
}

auto ManagedBuffer::getValidToFrame() const -> std::optional<uint64_t> {
  return validToFrame;
}

}
