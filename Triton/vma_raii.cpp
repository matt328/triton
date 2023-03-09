#include "vma_raii.h"

namespace vma {
   namespace raii {

      Allocator::Allocator(const vma::AllocatorCreateInfo& createInfo) {
         allocator = vma::createAllocator(createInfo);
      }

      Allocator::~Allocator() {
         Log::core->debug("Destroying Allocator");
         allocator.destroy();
      }

      std::unique_ptr<AllocatedBuffer> Allocator::createBuffer(const vk::BufferCreateInfo* bci,
                                                               const AllocationCreateInfo* aci,
                                                               const std::string_view& name) const {

         auto [buffer, allocation] = allocator.createBuffer(*bci, *aci);
         allocator.setAllocationName(allocation, name.data());

         auto ptr =
             std::make_unique<AllocatedBuffer>(allocator, std::move(buffer), std::move(allocation));

         return ptr;
      }

      void* Allocator::mapMemory(const AllocatedBuffer& allocatedBuffer) const {
         return allocator.mapMemory(allocatedBuffer.getAllocation());
      }

      void Allocator::unmapMemory(const AllocatedBuffer& allocatedBuffer) const {
         return allocator.unmapMemory(allocatedBuffer.getAllocation());
      }
   }
}
