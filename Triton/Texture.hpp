#pragma once

#include "ImmediateContext.hpp"
#include "vma_raii.hpp"

class Texture final {
 public:
   explicit Texture(const std::string_view& filename,
                    const vma::raii::Allocator& raiillocator,
                    const vk::raii::Device& device,
                    const ImmediateContext& graphicsContext,
                    const ImmediateContext& transferContext);
   ~Texture();
   vk::DescriptorImageInfo getDescriptorImageInfo() const;

 protected:
   std::unique_ptr<vma::raii::AllocatedImage> image;
   std::unique_ptr<vk::raii::ImageView> view;

   vk::ImageLayout imageLayout;

   std::unique_ptr<vk::raii::Sampler> sampler;

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
