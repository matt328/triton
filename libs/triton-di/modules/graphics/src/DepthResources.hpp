#pragma once

namespace tr::gfx::mem {
   class Image;
}

namespace tr::gfx {

   class IGraphicsDevice;

   class DepthResources {
    public:
      explicit DepthResources(const std::shared_ptr<IGraphicsDevice>& graphicsDevice);
      ~DepthResources();

      DepthResources(const DepthResources&) = default;
      DepthResources(DepthResources&&) = delete;
      auto operator=(const DepthResources&) -> DepthResources& = default;
      auto operator=(DepthResources&&) -> DepthResources& = delete;

      [[nodiscard]] auto getImageView() {
         return depthImageView;
      }

    private:
      std::shared_ptr<mem::Image> depthImage;
      std::shared_ptr<vk::raii::ImageView> depthImageView;
   };
}
