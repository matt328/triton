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

      std::unique_ptr<AllocatedImage> Allocator::createImage(
          const vk::ImageCreateInfo& imageCreateInfo,
          const vma::AllocationCreateInfo& allocationCreateInfo,
          const std::string_view& newName) const {

         auto [image, allocation] = allocator.createImage(imageCreateInfo, allocationCreateInfo);
         allocator.setAllocationName(allocation, newName.data());

         return std::make_unique<AllocatedImage>(
             allocator, std::move(image), std::move(allocation));
      }

      void* Allocator::mapMemory(const AllocatedBuffer& allocatedBuffer) const {
         return allocator.mapMemory(allocatedBuffer.getAllocation());
      }

      void Allocator::unmapMemory(const AllocatedBuffer& allocatedBuffer) const {
         return allocator.unmapMemory(allocatedBuffer.getAllocation());
      }

      void* Allocator::mapMemory(const AllocatedImage& allocatedImage) const {
         return allocator.mapMemory(allocatedImage.getAllocation());
      }

      void Allocator::unmapMemory(const AllocatedImage& allocatedImage) const {
         allocator.unmapMemory(allocatedImage.getAllocation());
      }
   }
}
