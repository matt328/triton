#pragma once

#include <vulkan-memory-allocator-hpp/vk_mem_alloc.hpp>

#include "Log.h"

namespace vma {
   namespace raii {

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

         std::unique_ptr<AllocatedBuffer> createBuffer(
             const vk::BufferCreateInfo* bci,
             const vma::AllocationCreateInfo* aci,
             const std::string_view& name = "unnamed buffer") const;

         void* mapMemory(const AllocatedBuffer& allocatedBuffer) const;
         void unmapMemory(const AllocatedBuffer& allocatedBuffer) const;

       private:
         vma::Allocator allocator;
      };
   }
}
