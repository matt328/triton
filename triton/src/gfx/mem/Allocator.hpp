#pragma once

namespace tr::gfx {
   class GraphicsDevice;
}

namespace tr::gfx::mem {

   class Buffer;
   class Image;

   class Allocator {
    public:
      explicit Allocator(const vma::AllocatorCreateInfo& createInfo,
                         const vk::raii::Device& device);
      ~Allocator();

      Allocator(const Allocator&) = delete;
      Allocator& operator=(const Allocator&) = delete;

      Allocator(Allocator&&) = delete;
      Allocator& operator=(Allocator&&) = delete;

      std::unique_ptr<Buffer> createBuffer(const vk::BufferCreateInfo* bci,
                                           const vma::AllocationCreateInfo* aci,
                                           const std::string_view& name = "unnamed buffer") const;

      [[nodiscard]] std::unique_ptr<Buffer> createDescriptorBuffer(
          size_t size,
          const std::string_view& name = "unnamed descriptor buffer") const;

      [[nodiscard]] std::unique_ptr<Buffer> createStagingBuffer(
          size_t size,
          const std::string_view& name = "unnamed buffer") const;

      [[nodiscard]] std::unique_ptr<Buffer> createGpuVertexBuffer(
          size_t size,
          const std::string_view& name = "unnamed buffer") const;

      [[nodiscard]] std::unique_ptr<Image> createImage(
          const vk::ImageCreateInfo& imageCreateInfo,
          const vma::AllocationCreateInfo& allocationCreateInfo,
          const std::string_view& newName = "unnamed image") const;

      [[nodiscard]] std::unique_ptr<Buffer> createGpuIndexBuffer(
          size_t size,
          const std::string_view& name) const;

      [[nodiscard]] void* mapMemory(const Buffer& Buffer) const;
      void unmapMemory(const Buffer& Buffer) const;

      [[nodiscard]] void* mapMemory(const Image& Image) const;
      void unmapMemory(const Image& Image) const;

    private:
      const vk::raii::Device& device;
      vma::Allocator allocator;
   };
}