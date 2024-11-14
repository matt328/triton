#pragma once

namespace tr::gfx {
   class VkContext;
   namespace mem {
      class Allocator;
      class Image;
   }
}

namespace tr::gfx::tex {

   using TransitionBarrierInfo =
       std::tuple<vk::ImageMemoryBarrier, vk::PipelineStageFlagBits, vk::PipelineStageFlagBits>;

   class Texture final {
    public:
      explicit Texture(const std::string_view& filename,
                       const mem::Allocator& allocator,
                       const vk::raii::Device& device,
                       const VkContext& transferContext);

      explicit Texture(void* data,
                       uint32_t width,
                       uint32_t height,
                       uint32_t channels,
                       const mem::Allocator& allocator,
                       const vk::raii::Device& device,
                       const VkContext& transferContext);

      ~Texture();

      Texture(const Texture&) = delete;
      Texture(Texture&&) = delete;
      auto operator=(const Texture&) -> Texture& = delete;
      auto operator=(Texture&&) -> Texture& = delete;

      [[nodiscard]] auto getImageInfo() const -> vk::DescriptorImageInfo {
         return imageInfo;
      }

      [[nodiscard]] auto getImageInfoRef() -> vk::DescriptorImageInfo& {
         return imageInfo;
      }

    private:
      std::unique_ptr<mem::Image> image;
      std::unique_ptr<vk::raii::ImageView> view;
      std::unique_ptr<vk::raii::Sampler> sampler;

      vk::ImageLayout imageLayout;
      vk::DescriptorImageInfo imageInfo;

      void initialize(void* data,
                      uint32_t width,
                      uint32_t height,
                      uint32_t channels,
                      const mem::Allocator& allocator,
                      const vk::raii::Device& device,
                      const VkContext& transferContext,
                      const std::string_view& textureName = "unnamed texture");

      static auto createTransitionBarrier(const vk::Image& image,
                                          vk::ImageLayout oldLayout,
                                          vk::ImageLayout newLayout,
                                          const vk::ImageSubresourceRange& subresourceRange)
          -> TransitionBarrierInfo;
   };
}
