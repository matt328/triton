#pragma once

#include <vulkan-memory-allocator-hpp/vk_mem_alloc.hpp>

#include "Log.h"

namespace vma {
   namespace raii {

      class AllocatedImage {
       public:
         AllocatedImage(const vma::Allocator& newAllocator,
                        const vk::Image newImage,
                        const vma::Allocation newAllocation)
             : image(newImage)
             , allocation(newAllocation)
             , allocator(newAllocator) {
         }

         ~AllocatedImage() {
            allocator.destroyImage(image, allocation);
         }

         const vk::Image& getImage() const {
            return image;
         }

         const vma::Allocation& getAllocation() const {
            return allocation;
         }

       private:
         vk::Image image;
         vma::Allocation allocation;
         vma::Allocator allocator;
      };

      class AllocatedBuffer {
       public:
         AllocatedBuffer(const vma::Allocator& newAllocator,
                         const vk::Buffer newBuffer,
                         const vma::Allocation newAllocation)
             : buffer(newBuffer)
             , allocation(newAllocation)
             , allocator(newAllocator) {
            Log::core->debug("Created Allocated Buffer");
         }

         ~AllocatedBuffer() {
            Log::core->debug("Destroyed buffer and allocation");
            allocator.destroyBuffer(buffer, allocation);
         }

         void updateBufferValue(void* data, const size_t dataSize) const {
            const auto src = allocator.mapMemory(allocation);
            memcpy(src, &data, dataSize);
            allocator.unmapMemory(allocation);
         }

         const vk::Buffer& getBuffer() const {
            return buffer;
         }

         const vma::Allocation& getAllocation() const {
            return allocation;
         }

       private:
         vk::Buffer buffer;
         vma::Allocation allocation;
         vma::Allocator allocator;
      };

      class Allocator {
       public:
         explicit Allocator(const vma::AllocatorCreateInfo& createInfo);
         ~Allocator();

         std::unique_ptr<AllocatedBuffer>&& createBuffer(
             const vk::BufferCreateInfo* bci,
             const vma::AllocationCreateInfo* aci,
             const std::string_view& name = "unnamed buffer") const;

         std::unique_ptr<AllocatedBuffer>&& createStagingBuffer(
             size_t size, const std::string_view& name = "unnamed buffer") const;

         std::unique_ptr<AllocatedBuffer>&& createGpuVertexBuffer(
             size_t size, const std::string_view& name = "unnamed buffer") const;

         std::unique_ptr<AllocatedImage>&& createImage(
             const vk::ImageCreateInfo& imageCreateInfo,
             const vma::AllocationCreateInfo& allocationCreateInfo,
             const std::string_view& newName = "unnamed image") const;

         std::unique_ptr<AllocatedBuffer>&& createGpuIndexBuffer(
             size_t size, const std::string_view& name) const;

         void* mapMemory(const AllocatedBuffer& allocatedBuffer) const;
         void unmapMemory(const AllocatedBuffer& allocatedBuffer) const;

         void* mapMemory(const AllocatedImage& allocatedImage) const;
         void unmapMemory(const AllocatedImage& allocatedImage) const;

       private:
         vma::Allocator allocator;
      };
   }
}
