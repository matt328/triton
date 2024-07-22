#pragma once

namespace tr::gfx {
   class GraphicsDevice;
}

namespace tr::gfx::mem {

   class AllocationException : public std::runtime_error {
    public:
      explicit AllocationException(const std::string& message) : std::runtime_error(message) {
      }
   };

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

      /// Creates a Buffer
      /// @throws AllocationException if there is an error allocating or naming the buffer.
      std::unique_ptr<Buffer> createBuffer(const vk::BufferCreateInfo* bci,
                                           const vma::AllocationCreateInfo* aci,
                                           const std::string_view& name = "unnamed buffer") const;

      /// Creates a Descriptor Buffer
      /// @throws AllocationException if there is an error allocating or naming the buffer.
      [[nodiscard]] std::unique_ptr<Buffer> createDescriptorBuffer(
          size_t size,
          const std::string_view& name = "unnamed descriptor buffer") const;

      /// Creates a Staging Buffer
      /// @throws AllocationException if there is an error allocating or naming the buffer.
      [[nodiscard]] std::unique_ptr<Buffer> createStagingBuffer(
          size_t size,
          const std::string_view& name = "unnamed buffer") const;

      /// Creates a GPU-Only Vertex Buffer
      /// @throws AllocationException if there is an error allocating or naming the buffer.
      [[nodiscard]] std::unique_ptr<Buffer> createGpuVertexBuffer(
          size_t size,
          const std::string_view& name = "unnamed buffer") const;

      /// Creates an Image
      /// @throws AllocationException if there is an error allocating or naming the buffer.
      [[nodiscard]] std::unique_ptr<Image> createImage(
          const vk::ImageCreateInfo& imageCreateInfo,
          const vma::AllocationCreateInfo& allocationCreateInfo,
          const std::string_view& newName = "unnamed image") const;

      /// Creates a GPU-Only Index Buffer
      /// @throws AllocationException if there is an error allocating or naming the buffer.
      [[nodiscard]] std::unique_ptr<Buffer> createGpuIndexBuffer(
          size_t size,
          const std::string_view& name) const;

      /// Maps the memory used by the given buffer
      /// @returns A void pointer pointing to the memory
      /// @throws AllocationException if there is an error mapping the memory
      [[nodiscard]] void* mapMemory(const Buffer& Buffer) const;

      /// Unmaps the memory used by the given buffer
      /// @throws AllocationException if there is an error mapping the memory
      void unmapMemory(const Buffer& Buffer) const;

      /// Maps the memory used by the given Image
      /// @returns A void pointer pointing to the memory
      /// @throws AllocationException if there is an error mapping the memory
      [[nodiscard]] void* mapMemory(const Image& Image) const;

      /// Unmaps the memory used by the given image
      /// @throws AllocationException if there is an error mapping the memory
      void unmapMemory(const Image& Image) const;

    private:
      const vk::raii::Device& device;
      vma::Allocator allocator;
   };
}