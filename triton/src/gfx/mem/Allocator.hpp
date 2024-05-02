#pragma once

namespace tr::gfx::mem {

   class AllocatedBuffer;
   class AllocatedImage;

   class Allocator {
    public:
      explicit Allocator(const vma::AllocatorCreateInfo& createInfo);
      ~Allocator();

      Allocator(const Allocator&) = delete;
      Allocator& operator=(const Allocator&) = delete;

      Allocator(Allocator&&) = delete;
      Allocator& operator=(Allocator&&) = delete;

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
      const vk::Device& device;
      vma::Allocator allocator;
   };
}