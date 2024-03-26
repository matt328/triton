#pragma once

#include "../vma_raii.hpp"
#include "gfx/VkContext.hpp"
#include <vulkan/vulkan_structs.hpp>

namespace tr::gfx::Textures {

   using TransitionBarrierInfo =
       std::tuple<vk::ImageMemoryBarrier, vk::PipelineStageFlagBits, vk::PipelineStageFlagBits>;

   class Texture final {
    public:
      explicit Texture(const std::string_view& filename,
                       const Allocator& raiillocator,
                       const vk::raii::Device& device,
                       const VkContext& transferContext);

      explicit Texture(void* data,
                       uint32_t width,
                       uint32_t height,
                       uint32_t channels,
                       const Allocator& raiillocator,
                       const vk::raii::Device& device,
                       const VkContext& transferContext);

      ~Texture() = default;

      Texture(const Texture&) = delete;
      Texture(Texture&&) = delete;
      Texture& operator=(const Texture&) = delete;
      Texture& operator=(Texture&&) = delete;

      [[nodiscard]] const vk::DescriptorImageInfo getImageInfo() const {
         return imageInfo;
      }

      [[nodiscard]] vk::DescriptorImageInfo& getImageInfoRef() {
         return imageInfo;
      }

    private:
      std::unique_ptr<AllocatedImage> image;
      std::unique_ptr<vk::raii::ImageView> view;
      std::unique_ptr<vk::raii::Sampler> sampler;

      vk::ImageLayout imageLayout;
      vk::DescriptorImageInfo imageInfo;

      void initialize(void* data,
                      uint32_t width,
                      uint32_t height,
                      uint32_t channels,
                      const Allocator& raiillocator,
                      const vk::raii::Device& device,
                      const VkContext& transferContext,
                      const std::string_view& textureName = "unnamed texture");

      static TransitionBarrierInfo createTransitionBarrier(
          const vk::Image& image,
          const vk::ImageLayout oldLayout,
          const vk::ImageLayout newLayout,
          const vk::ImageSubresourceRange subresourceRange);
   };
}