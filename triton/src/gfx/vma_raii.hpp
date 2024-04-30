#pragma once

namespace tr::gfx {

   class AllocatedImage {
    public:
      AllocatedImage(const AllocatedImage&) = default;
      AllocatedImage(AllocatedImage&&) = delete;
      AllocatedImage& operator=(const AllocatedImage&) = default;
      AllocatedImage& operator=(AllocatedImage&&) = delete;

      AllocatedImage(const vma::Allocator& newAllocator,
                     const vk::Image newImage,
                     const vma::Allocation newAllocation)
          : image(newImage), allocation(newAllocation), allocator(newAllocator) {
      }

      ~AllocatedImage() {
         allocator.destroyImage(image, allocation);
      }

      [[nodiscard]] const vk::Image& getImage() const {
         return image;
      }

      [[nodiscard]] const vma::Allocation& getAllocation() const {
         return allocation;
      }

    private:
      vk::Image image;
      vma::Allocation allocation;
      vma::Allocator allocator;
   };

   class AllocatedBuffer {
    public:
      AllocatedBuffer(const AllocatedBuffer&) = default;
      AllocatedBuffer(AllocatedBuffer&&) = delete;
      AllocatedBuffer& operator=(const AllocatedBuffer&) = default;
      AllocatedBuffer& operator=(AllocatedBuffer&&) = delete;

      AllocatedBuffer(const vma::Allocator& newAllocator,
                      const vk::Buffer newBuffer,
                      const vk::DeviceSize range,
                      const vma::Allocation newAllocation)
          : buffer(newBuffer),
            bufferInfo{vk::DescriptorBufferInfo{.buffer = newBuffer, .offset = 0, .range = range}},
            allocation(newAllocation),
            allocator(newAllocator) {
      }

      ~AllocatedBuffer() {
         if (isMapped) {
            unmapBuffer();
         }
         allocator.destroyBuffer(buffer, allocation);
      }

      /// Permanently maps the buffer, until either unmapBuffer() is called, or the object is
      /// destroyed.  Meant to be paired with updateMappedBufferValue()
      void mapBuffer() {
         mappedMemory = allocator.mapMemory(allocation);
         isMapped = true;
      }

      /// Copy some data into this buffer. Don't forget to mapBuffer() first. Leaves the buffer
      /// mapped. Either call unmapBuffer() or let the object go out of scope.
      void updateMappedBufferValue(const void* data, const size_t dataSize) const {
         assert(isMapped);
         memcpy(mappedMemory, data, dataSize);
      }

      // Unmap this buffer. Only call this after you've called mapBuffer() and when you're done
      // copying data into it for awhile.
      void unmapBuffer() {
         assert(isMapped);
         allocator.unmapMemory(allocation);
         isMapped = false;
      }

      /// Maps, memcpy's and then unmaps the buffer
      void updateBufferValue(const void* data, const size_t dataSize) const {
         auto dst = allocator.mapMemory(allocation);
         memcpy(dst, data, dataSize);
         allocator.unmapMemory(allocation);
      }

      [[nodiscard]] const vk::Buffer& getBuffer() const {
         return buffer;
      }

      [[nodiscard]] const vma::Allocation& getAllocation() const {
         return allocation;
      }

      [[nodiscard]] vk::DescriptorBufferInfo* getBufferInfo() {
         return &bufferInfo;
      }

    private:
      void* mappedMemory{};
      bool isMapped{};
      vk::Buffer buffer;
      vk::DescriptorBufferInfo bufferInfo;
      vma::Allocation allocation;
      vma::Allocator allocator;
   };

   class Allocator {
    public:
      explicit Allocator(const vma::AllocatorCreateInfo& createInfo);

      Allocator(const Allocator&) = default;
      Allocator(Allocator&&) = delete;
      Allocator& operator=(const Allocator&) = default;
      Allocator& operator=(Allocator&&) = delete;
      ~Allocator();

      std::unique_ptr<AllocatedBuffer> createBuffer(
          const vk::BufferCreateInfo* bci,
          const vma::AllocationCreateInfo* aci,
          const std::string_view& name = "unnamed buffer") const;

      [[nodiscard]] std::unique_ptr<AllocatedBuffer> createStagingBuffer(
          size_t size,
          const std::string_view& name = "unnamed buffer") const;

      [[nodiscard]] std::unique_ptr<AllocatedBuffer> createGpuVertexBuffer(
          size_t size,
          const std::string_view& name = "unnamed buffer") const;

      [[nodiscard]] std::unique_ptr<AllocatedImage> createImage(
          const vk::ImageCreateInfo& imageCreateInfo,
          const vma::AllocationCreateInfo& allocationCreateInfo,
          const std::string_view& newName = "unnamed image") const;

      [[nodiscard]] std::unique_ptr<AllocatedBuffer> createGpuIndexBuffer(
          size_t size,
          const std::string_view& name) const;

      [[nodiscard]] void* mapMemory(const AllocatedBuffer& allocatedBuffer) const;
      void unmapMemory(const AllocatedBuffer& allocatedBuffer) const;

      [[nodiscard]] void* mapMemory(const AllocatedImage& allocatedImage) const;
      void unmapMemory(const AllocatedImage& allocatedImage) const;

    private:
      vma::Allocator allocator;
   };
}
