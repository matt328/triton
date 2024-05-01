#include "vma_raii.hpp"

#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>
#include <vulkan-memory-allocator-hpp/vk_mem_alloc.hpp>

namespace tr::gfx {

   Allocator::Allocator(const vma::AllocatorCreateInfo& createInfo) : device{createInfo.device} {
      allocator = createAllocator(createInfo);
   }

   Allocator::~Allocator() {
      Log::debug << "Destroying Allocator" << std::endl;
      allocator.destroy();
   }

   std::unique_ptr<AllocatedBuffer> Allocator::createBuffer(const vk::BufferCreateInfo* bci,
                                                            const vma::AllocationCreateInfo* aci,
                                                            const std::string_view& name) const {

      auto [buffer, allocation] = allocator.createBuffer(*bci, *aci);
      allocator.setAllocationName(allocation, name.data());

      return std::make_unique<AllocatedBuffer>(allocator, buffer, bci->size, allocation, device);
   }

   std::unique_ptr<AllocatedBuffer> Allocator::createStagingBuffer(
       const size_t size,
       const std::string_view& name) const {

      const auto bufferCreateInfo =
          vk::BufferCreateInfo{.size = size,
                               .usage = vk::BufferUsageFlagBits::eTransferSrc,
                               .sharingMode = vk::SharingMode::eExclusive};
      const auto allocationCreateInfo =
          vma::AllocationCreateInfo{.usage = vma::MemoryUsage::eCpuOnly,
                                    .requiredFlags = vk::MemoryPropertyFlagBits::eHostVisible |
                                                     vk::MemoryPropertyFlagBits::eHostCoherent};

      return createBuffer(&bufferCreateInfo, &allocationCreateInfo, name);
   }

   std::unique_ptr<AllocatedBuffer> Allocator::createGpuVertexBuffer(
       const size_t size,
       const std::string_view& name) const {

      const auto bufferCreateInfo = vk::BufferCreateInfo{
          .size = size,
          .usage = vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst,
          .sharingMode = vk::SharingMode::eExclusive};
      constexpr auto allocationCreateInfo =
          vma::AllocationCreateInfo{.usage = vma::MemoryUsage::eGpuOnly};

      return createBuffer(&bufferCreateInfo, &allocationCreateInfo, name);
   }

   std::unique_ptr<AllocatedImage> Allocator::createImage(
       const vk::ImageCreateInfo& imageCreateInfo,
       const vma::AllocationCreateInfo& allocationCreateInfo,
       const std::string_view& newName) const {

      auto [image, allocation] = allocator.createImage(imageCreateInfo, allocationCreateInfo);
      allocator.setAllocationName(allocation, newName.data());
      return std::make_unique<AllocatedImage>(allocator, std::move(image), std::move(allocation));
   }

   std::unique_ptr<AllocatedBuffer> Allocator::createGpuIndexBuffer(
       const size_t size,
       const std::string_view& name) const {
      const auto bufferCreateInfo = vk::BufferCreateInfo{
          .size = size,
          .usage = vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst,
          .sharingMode = vk::SharingMode::eExclusive};
      constexpr auto allocationCreateInfo =
          vma::AllocationCreateInfo{.usage = vma::MemoryUsage::eGpuOnly};

      return createBuffer(&bufferCreateInfo, &allocationCreateInfo, name);
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
