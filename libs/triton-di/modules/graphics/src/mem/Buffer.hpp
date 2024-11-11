#pragma once

namespace tr::gfx::mem {
   class Buffer {
    public:
      Buffer(const vma::Allocator& newAllocator,
             vk::Buffer newBuffer,
             vk::DeviceSize range,
             vma::Allocation newAllocation,
             const vk::Device& device,
             const vma::AllocationInfo& allocationInfo) noexcept;

      ~Buffer();

      Buffer(const Buffer&) = delete;
      auto operator=(const Buffer&) -> Buffer& = delete;

      Buffer(Buffer&&) = delete;
      auto operator=(Buffer&&) -> Buffer& = delete;

      /// Permanently maps the buffer, until either unmapBuffer() is called, or the object is
      /// destroyed.  Meant to be paired with updateMappedBufferValue()
      void mapBuffer();

      /// Copy some data into this buffer. Don't forget to mapBuffer() first. Leaves the buffer
      /// mapped. Either call unmapBuffer() or let the object go out of scope.
      void updateMappedBufferValue(const void* data, size_t dataSize) const;

      // Unmap this buffer. Only call this after you've called mapBuffer() and when you're done
      // copying data into it for awhile.
      void unmapBuffer();

      /// Maps, memcpy's and then unmaps the buffer
      void updateBufferValue(const void* data, size_t dataSize) const;

      [[nodiscard]] auto getBuffer() const -> const vk::Buffer& {
         return buffer;
      }

      [[nodiscard]] auto getAllocation() const -> const vma::Allocation& {
         return allocation;
      }

      [[nodiscard]] auto getBufferInfo() -> vk::DescriptorBufferInfo* {
         return &bufferInfo;
      }

      [[nodiscard]] auto getData() const {
         return allocationInfo.pMappedData;
      }

      [[nodiscard]] auto getDeviceAddress() const -> uint64_t;

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