#include "ManagedBuffer.hpp"

namespace tr {

ManagedBuffer::ManagedBuffer(vk::Buffer newVkBuffer,
                             BufferMeta newBufferMeta,
                             std::shared_ptr<vma::Allocator> newAllocator,
                             vma::Allocation newAllocation)
    : vkBuffer{newVkBuffer},
      bufferMeta{std::move(newBufferMeta)},
      allocator{std::move(newAllocator)},
      allocation{newAllocation} {
  Log.trace("Creating Managed Buffer");
}

ManagedBuffer::~ManagedBuffer() {
  Log.trace("Destroying ManagedBuffer");
  allocator->destroyBuffer(vkBuffer, allocation);
}

auto ManagedBuffer::isMappable() -> bool {
  const auto memProps = allocator->getAllocationMemoryProperties(allocation);
  return (memProps & vk::MemoryPropertyFlagBits::eHostCoherent) != vk::MemoryPropertyFlagBits{};
}

[[nodiscard]] auto ManagedBuffer::getVkBuffer() const -> const vk::Buffer& {
  return vkBuffer;
}

[[nodiscard]] auto ManagedBuffer::getMeta() const -> const BufferMeta& {
  return bufferMeta;
}

}
