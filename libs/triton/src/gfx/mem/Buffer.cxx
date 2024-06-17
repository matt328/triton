#include "Buffer.hpp"

namespace tr::gfx::mem {
   Buffer::Buffer(const vma::Allocator& newAllocator,
                  const vk::Buffer newBuffer,
                  const vk::DeviceSize range,
                  const vma::Allocation newAllocation,
                  const vk::Device& device,
                  const vma::AllocationInfo allocationInfo)
       : device{device},
         buffer(newBuffer),
         bufferInfo{vk::DescriptorBufferInfo{.buffer = newBuffer, .offset = 0, .range = range}},
         allocation(newAllocation),
         allocator(newAllocator),
         allocationInfo{allocationInfo} {
   }

   Buffer::~Buffer() {
      if (isMapped) {
         unmapBuffer();
      }
      allocator.destroyBuffer(buffer, allocation);
   }

   void Buffer::mapBuffer() {
      mappedMemory = allocator.mapMemory(allocation);
      isMapped = true;
   }

   void Buffer::updateMappedBufferValue(const void* data, const size_t dataSize) const {
      assert(isMapped);
      memcpy(mappedMemory, data, dataSize);
   }

   void Buffer::unmapBuffer() {
      assert(isMapped);
      allocator.unmapMemory(allocation);
      isMapped = false;
   }

   void Buffer::updateBufferValue(const void* data, const size_t dataSize) const {
      auto dst = allocator.mapMemory(allocation);
      memcpy(dst, data, dataSize);
      allocator.unmapMemory(allocation);
   }

   [[nodiscard]] uint64_t Buffer::getDeviceAddress() const {
      const auto bdai = vk::BufferDeviceAddressInfoKHR{.buffer = buffer};
      return device.getBufferAddress(bdai);
   }
}