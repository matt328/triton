#include "Allocator.hpp"
#include "Buffer.hpp"
#include "Image.hpp"

#include "gfx/GraphicsDevice.hpp"
#include <vulkan-memory-allocator-hpp/vk_mem_alloc_enums.hpp>

#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>
#include <vulkan-memory-allocator-hpp/vk_mem_alloc.hpp>

namespace tr::gfx::mem {
   Allocator::Allocator(const vma::AllocatorCreateInfo& createInfo, const vk::raii::Device& device)
       : device{device} {
      allocator = createAllocator(createInfo);
   }

   Allocator::~Allocator() {
      Log::debug << "Destroying Allocator" << std::endl;
      allocator.destroy();
   }

   std::unique_ptr<Buffer> Allocator::createBuffer(const vk::BufferCreateInfo* bci,
                                                   const vma::AllocationCreateInfo* aci,
                                                   const std::string_view& name) const {

      vma::AllocationInfo info{};
      auto [buffer, allocation] = allocator.createBuffer(*bci, *aci, info);
      allocator.setAllocationName(allocation, name.data());

      return std::make_unique<Buffer>(allocator, buffer, bci->size, allocation, *device, info);
   }

   std::unique_ptr<Buffer> Allocator::createDescriptorBuffer(size_t size,
                                                             const std::string_view& name) const {
      const auto bci =
          vk::BufferCreateInfo{.size = size,
                               .usage = vk::BufferUsageFlagBits::eResourceDescriptorBufferEXT |
                                        vk::BufferUsageFlagBits::eShaderDeviceAddress};

      const auto aci =
          vma::AllocationCreateInfo{.flags = vma::AllocationCreateFlagBits::eMapped |
                                             vma::AllocationCreateFlagBits::eHostAccessRandom,
                                    .usage = vma::MemoryUsage::eCpuToGpu,
                                    .requiredFlags = vk::MemoryPropertyFlagBits::eHostCoherent};
      return createBuffer(&bci, &aci, name);
   }

   std::unique_ptr<Buffer> Allocator::createStagingBuffer(const size_t size,
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

   std::unique_ptr<Buffer> Allocator::createGpuVertexBuffer(const size_t size,
                                                            const std::string_view& name) const {

      const auto bufferCreateInfo = vk::BufferCreateInfo{
          .size = size,
          .usage = vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst,
          .sharingMode = vk::SharingMode::eExclusive};
      constexpr auto allocationCreateInfo =
          vma::AllocationCreateInfo{.usage = vma::MemoryUsage::eGpuOnly};

      return createBuffer(&bufferCreateInfo, &allocationCreateInfo, name);
   }

   std::unique_ptr<Image> Allocator::createImage(
       const vk::ImageCreateInfo& imageCreateInfo,
       const vma::AllocationCreateInfo& allocationCreateInfo,
       const std::string_view& newName) const {

      auto [image, allocation] = allocator.createImage(imageCreateInfo, allocationCreateInfo);
      allocator.setAllocationName(allocation, newName.data());
      return std::make_unique<Image>(allocator, std::move(image), std::move(allocation));
   }

   std::unique_ptr<Buffer> Allocator::createGpuIndexBuffer(const size_t size,
                                                           const std::string_view& name) const {
      const auto bufferCreateInfo = vk::BufferCreateInfo{
          .size = size,
          .usage = vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst,
          .sharingMode = vk::SharingMode::eExclusive};
      constexpr auto allocationCreateInfo =
          vma::AllocationCreateInfo{.usage = vma::MemoryUsage::eGpuOnly};

      return createBuffer(&bufferCreateInfo, &allocationCreateInfo, name);
   }

   void* Allocator::mapMemory(const Buffer& Buffer) const {
      return allocator.mapMemory(Buffer.getAllocation());
   }

   void Allocator::unmapMemory(const Buffer& Buffer) const {
      return allocator.unmapMemory(Buffer.getAllocation());
   }

   void* Allocator::mapMemory(const Image& Image) const {
      return allocator.mapMemory(Image.getAllocation());
   }

   void Allocator::unmapMemory(const Image& Image) const {
      allocator.unmapMemory(Image.getAllocation());
   }
}