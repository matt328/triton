#pragma once

namespace tr::gfx::mem {
   class Buffer {
    public:
      Buffer(const vma::Allocator& newAllocator,
             const vk::Buffer newBuffer,
             const vk::DeviceSize range,
             const vma::Allocation newAllocation,
             const vk::Device& device,
             const vma::AllocationInfo allocationInfo) noexcept;

      ~Buffer();

      Buffer(const Buffer&) = delete;
      Buffer& operator=(const Buffer&) = delete;

      Buffer(Buffer&&) = delete;
      Buffer& operator=(Buffer&&) = delete;

      /// Permanently maps the buffer, until either unmapBuffer() is called, or the object is
      /// destroyed.  Meant to be paired with updateMappedBufferValue()
      void mapBuffer();

      /// Copy some data into this buffer. Don't forget to mapBuffer() first. Leaves the buffer
      /// mapped. Either call unmapBuffer() or let the object go out of scope.
      void updateMappedBufferValue(const void* data, const size_t dataSize) const;

      // Unmap this buffer. Only call this after you've called mapBuffer() and when you're done
      // copying data into it for awhile.
      void unmapBuffer();

      /// Maps, memcpy's and then unmaps the buffer
      void updateBufferValue(const void* data, const size_t dataSize) const;

      [[nodiscard]] const vk::Buffer& getBuffer() const {
         return buffer;
      }

      [[nodiscard]] const vma::Allocation& getAllocation() const {
         return allocation;
      }

      [[nodiscard]] vk::DescriptorBufferInfo* getBufferInfo() {
         return &bufferInfo;
      }

      [[nodiscard]] auto getData() const {
         return allocationInfo.pMappedData;
      }

      [[nodiscard]] uint64_t getDeviceAddress() const;

    private:
      const vk::Device& device;
      void* mappedMemory{};
      bool isMapped{};
      vk::Buffer buffer;
      vk::DescriptorBufferInfo bufferInfo;
      vma::Allocation allocation;
      vma::Allocator allocator;
      vma::AllocationInfo allocationInfo;
   };
}