#include "IBuffer.hpp"
#include "vk/core/Device.hpp"

namespace tr {

IBuffer::IBuffer(std::shared_ptr<Device> newDevice,
                 vk::Buffer newVkBuffer,
                 std::shared_ptr<vma::Allocator> newAllocator,
                 vma::Allocation newAllocation)
    : device{std::move(newDevice)},
      vkBuffer{newVkBuffer},
      allocator{std::move(newAllocator)},
      allocation{newAllocation} {
}

auto IBuffer::getVkBuffer() const -> const vk::Buffer& {
  return vkBuffer;
}

auto IBuffer::getAddress() const -> uint64_t {
  return device->getVkDevice().getBufferAddress(vk::BufferDeviceAddressInfo{.buffer = vkBuffer});
}

}
