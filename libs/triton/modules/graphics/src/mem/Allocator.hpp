#pragma once

#include "tr/IDebugManager.hpp"
#include "vk/core/PhysicalDevice.hpp"
#include <vk_mem_alloc_handles.hpp>

namespace tr {

class AllocationException : public std::runtime_error {
public:
  explicit AllocationException(const std::string& message) : std::runtime_error(message) {
  }
};

class Buffer;
class Image;

class Allocator {
public:
  Allocator(std::shared_ptr<Device> newDevice,
            const std::shared_ptr<PhysicalDevice>& physicalDevice,
            const std::shared_ptr<Instance>& instance,
            std::shared_ptr<IDebugManager> newDebugManager);
  ~Allocator();

  Allocator(const Allocator&) = delete;
  auto operator=(const Allocator&) -> Allocator& = delete;

  Allocator(Allocator&&) = delete;
  auto operator=(Allocator&&) -> Allocator& = delete;

  /// Creates a Buffer
  /// @throws AllocationException if there is an error allocating or naming the buffer.
  auto createBuffer(const vk::BufferCreateInfo* bci,
                    const vma::AllocationCreateInfo* aci,
                    const std::string_view& name = "unnamed buffer") const
      -> std::unique_ptr<Buffer>;

  /// Creates a Descriptor Buffer
  /// @throws AllocationException if there is an error allocating or naming the buffer.
  [[nodiscard]] auto createDescriptorBuffer(
      size_t size,
      const std::string_view& name = "unnamed descriptor buffer") const -> std::unique_ptr<Buffer>;

  /// Creates a Staging Buffer
  /// @throws AllocationException if there is an error allocating or naming the buffer.
  [[nodiscard]] auto createStagingBuffer(size_t size,
                                         const std::string_view& name = "unnamed buffer") const
      -> std::unique_ptr<Buffer>;

  /// Creates a GPU-Only Vertex Buffer
  /// @throws AllocationException if there is an error allocating or naming the buffer.
  [[nodiscard]] auto createGpuVertexBuffer(size_t size,
                                           const std::string_view& name = "unnamed buffer") const
      -> std::unique_ptr<Buffer>;

  /// Creates an Image
  /// @throws AllocationException if there is an error allocating or naming the buffer.
  [[nodiscard]] auto createImage(const vk::ImageCreateInfo& imageCreateInfo,
                                 const vma::AllocationCreateInfo& allocationCreateInfo,
                                 const std::string_view& newName = "unnamed image") const
      -> std::unique_ptr<Image>;

  /// Creates a GPU-Only Index Buffer
  /// @throws AllocationException if there is an error allocating or naming the buffer.
  [[nodiscard]] auto createGpuIndexBuffer(size_t size, const std::string_view& name) const
      -> std::unique_ptr<Buffer>;

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
  [[nodiscard]] auto mapMemory(const Image& Image) const -> void*;

  /// Unmaps the memory used by the given image
  /// @throws AllocationException if there is an error mapping the memory
  void unmapMemory(const Image& Image) const;

  auto getAllocator() {
    return allocator;
  }

private:
  std::shared_ptr<Device> device;
  std::shared_ptr<vma::Allocator> allocator;
  std::shared_ptr<IDebugManager> debugManager;
};
}
