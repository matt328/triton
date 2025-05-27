#include "ManagedBuffer.hpp"

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
    if (mappedData != nullptr) {
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
  if (mappedData == nullptr) {
    mappedData = allocator->mapMemory(allocation);
  }
}

auto ManagedBuffer::uploadData(const void* srcData, size_t size, size_t offset) -> void {
  if (mappedData == nullptr) {
    map();
  }
  std::memcpy(static_cast<char*>(mappedData) + offset, srcData, size);
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
