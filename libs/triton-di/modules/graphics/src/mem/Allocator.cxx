#include "Allocator.hpp"
#include "Buffer.hpp"
#include "Image.hpp"

#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>
#include <vk_mem_alloc.hpp>

namespace tr::gfx::mem {

   Allocator::Allocator(const vma::AllocatorCreateInfo& createInfo, const vk::raii::Device& device)
       : device{device} {
      allocator = std::make_shared<vma::Allocator>(createAllocator(createInfo));
   }

   Allocator::~Allocator() {
      Log.debug("Destroying Allocator");
      allocator->destroy();
   }

   auto Allocator::createBuffer(const vk::BufferCreateInfo* bci,
                                const vma::AllocationCreateInfo* aci,
                                const std::string_view& name) const -> std::unique_ptr<Buffer> {

      vma::AllocationInfo info{};
      try {
         auto [buffer, allocation] = allocator->createBuffer(*bci, *aci, info);
         allocator->setAllocationName(allocation, name.data());
         return std::make_unique<Buffer>(*allocator, buffer, bci->size, allocation, *device, info);
      } catch (const std::exception& ex) {
         throw AllocationException(
             fmt::format("Error creating and/or naming Buffer: {0}, {1}", name, ex.what()));
      }
   }

   auto Allocator::createDescriptorBuffer(const size_t size, const std::string_view& name) const
       -> std::unique_ptr<Buffer> {
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

   auto Allocator::createStagingBuffer(const size_t size, const std::string_view& name) const
       -> std::unique_ptr<Buffer> {

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

   auto Allocator::createGpuVertexBuffer(const size_t size, const std::string_view& name) const
       -> std::unique_ptr<Buffer> {

      const auto bufferCreateInfo = vk::BufferCreateInfo{
          .size = size,
          .usage = vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst,
          .sharingMode = vk::SharingMode::eExclusive};
      constexpr auto allocationCreateInfo =
          vma::AllocationCreateInfo{.usage = vma::MemoryUsage::eGpuOnly};

      return createBuffer(&bufferCreateInfo, &allocationCreateInfo, name);
   }

   auto Allocator::createImage(const vk::ImageCreateInfo& imageCreateInfo,
                               const vma::AllocationCreateInfo& allocationCreateInfo,
                               const std::string_view& newName) const -> std::unique_ptr<Image> {

      try {
         auto [image, allocation] = allocator->createImage(imageCreateInfo, allocationCreateInfo);
         allocator->setAllocationName(allocation, newName.data());
         return std::make_unique<Image>(*allocator, image, allocation);
      } catch (const std::exception& ex) {
         throw AllocationException(
             fmt::format("Error creating and/or naming Image: {0}, {1}", newName, ex.what()));
      }
   }

   auto Allocator::createGpuIndexBuffer(const size_t size, const std::string_view& name) const
       -> std::unique_ptr<Buffer> {
      const auto bufferCreateInfo = vk::BufferCreateInfo{
          .size = size,
          .usage = vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst,
          .sharingMode = vk::SharingMode::eExclusive};
      constexpr auto allocationCreateInfo =
          vma::AllocationCreateInfo{.usage = vma::MemoryUsage::eGpuOnly};

      return createBuffer(&bufferCreateInfo, &allocationCreateInfo, name);
   }

   auto Allocator::mapMemory(const Buffer& Buffer) const -> void* {
      try {
         return allocator->mapMemory(Buffer.getAllocation());
      } catch (const std::exception& ex) {
         throw AllocationException(fmt::format("Error mapping buffer: {0}", ex.what()));
      }
   }

   void Allocator::unmapMemory(const Buffer& Buffer) const {
      try {
         return allocator->unmapMemory(Buffer.getAllocation());
      } catch (const std::exception& ex) {
         throw AllocationException(fmt::format("Error ummapping buffer: {0}", ex.what()));
      }
   }

   auto Allocator::mapMemory(const Image& Image) const -> void* {
      try {
         return allocator->mapMemory(Image.getAllocation());
      } catch (const std::exception& ex) {
         throw AllocationException(fmt::format("Error mapping Image: {0}", ex.what()));
      }
   }

   void Allocator::unmapMemory(const Image& Image) const {
      try {
         allocator->unmapMemory(Image.getAllocation());
      } catch (const std::exception& ex) {
         throw AllocationException(fmt::format("Error unmapping Image: {0}", ex.what()));
      }
   }
}