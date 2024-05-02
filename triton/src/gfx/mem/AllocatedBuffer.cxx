#include "AllocatedBuffer.hpp"

namespace tr::gfx::mem {
   AllocatedBuffer::AllocatedBuffer(const vma::Allocator& newAllocator,
                                    const vk::Buffer newBuffer,
                                    const vk::DeviceSize range,
                                    const vma::Allocation newAllocation,
                                    const vk::Device& device)
       : device{device},
         buffer(newBuffer),
         bufferInfo{vk::DescriptorBufferInfo{.buffer = newBuffer, .offset = 0, .range = range}},
         allocation(newAllocation),
         allocator(newAllocator) {
   }

   AllocatedBuffer::~AllocatedBuffer() {
      if (isMapped) {
         unmapBuffer();
      }
      allocator.destroyBuffer(buffer, allocation);
   }

   void AllocatedBuffer::mapBuffer() {
      mappedMemory = allocator.mapMemory(allocation);
      isMapped = true;
   }

   void AllocatedBuffer::updateMappedBufferValue(const void* data, const size_t dataSize) const {
      assert(isMapped);
      memcpy(mappedMemory, data, dataSize);
   }

   void AllocatedBuffer::unmapBuffer() {
      assert(isMapped);
      allocator.unmapMemory(allocation);
      isMapped = false;
   }

   void AllocatedBuffer::updateBufferValue(const void* data, const size_t dataSize) const {
      auto dst = allocator.mapMemory(allocation);
      memcpy(dst, data, dataSize);
      allocator.unmapMemory(allocation);
   }

   [[nodiscard]] uint64_t AllocatedBuffer::getDeviceAddress() const {
      const auto bdai = vk::BufferDeviceAddressInfoKHR{.buffer = buffer};
      return device.getBufferAddressEXT(bdai);
   }
}