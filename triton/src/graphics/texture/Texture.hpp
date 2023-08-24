#pragma once

#include "graphics/ImmediateContext.hpp"
#include "core/vma_raii.hpp"
#include <vulkan/vulkan_raii.hpp>
#include <vulkan/vulkan_structs.hpp>

class Texture final {
 public:
   explicit Texture(const std::string_view& filename,
                    const vma::raii::Allocator& raiillocator,
                    const vk::raii::Device& device,
                    const ImmediateContext& graphicsContext,
                    const ImmediateContext& transferContext);
   ~Texture() = default;

   Texture(const Texture&) = delete;
   Texture(Texture&&) = delete;
   Texture& operator=(const Texture&) = delete;
   Texture& operator=(Texture&&) = delete;

   void updateDescriptorSet(const vk::raii::DescriptorSet& descriptorSet) const;

   const vk::DescriptorImageInfo* getImageInfo() {
      return &imageInfo;
   }

 private:
   const vk::raii::Device& device;
   std::unique_ptr<vma::raii::AllocatedImage> image;
   std::unique_ptr<vk::raii::ImageView> view;

   vk::ImageLayout imageLayout;

   std::unique_ptr<vk::raii::Sampler> sampler;
   vk::DescriptorImageInfo imageInfo;

   static void transitionImageLayout(const ImmediateContext& context,
                                     const vk::Image& image,
                                     vk::ImageLayout oldLayout,
                                     vk::ImageLayout newLayout,
                                     vk::ImageSubresourceRange subresourceRange);

   static void copyBufferToImage(const ImmediateContext& context,
                                 const vk::Buffer& buffer,
                                 const vk::Image& image,
                                 vk::ImageLayout imageLayout,
                                 const std::vector<vk::BufferImageCopy>& regions);
};
