#pragma once

#include "gfx/IRenderContext.hpp"
#include "sb/ILayoutFactory.hpp"

namespace tr::gfx {
   class IGraphicsDevice;
}

namespace tr::gfx::sb {

   class LayoutFactory : public ILayoutFactory {

    public:
      LayoutFactory(const std::shared_ptr<gfx::IGraphicsDevice>& graphicsDevice,
                    RenderContextConfig config);
      ~LayoutFactory() override;

      LayoutFactory(const LayoutFactory&) = delete;
      auto operator=(const LayoutFactory&) -> LayoutFactory& = delete;
      LayoutFactory(LayoutFactory&&) = delete;
      auto operator=(LayoutFactory&&) -> LayoutFactory& = delete;

      [[nodiscard]] auto getLayout(const LayoutHandle handle) const -> const Layout& override {
         return *layoutCache.at(handle);
      }

      [[nodiscard]] auto getVkLayout(LayoutHandle handle) const
          -> const vk::DescriptorSetLayout& override;

    private:
      RenderContextConfig config;

      void initBindlessLayout(std::shared_ptr<gfx::IGraphicsDevice> graphicsDevice);
      void initPerFrameLayout(std::shared_ptr<gfx::IGraphicsDevice> graphicsDevice);
      void initObjectDataLayout(std::shared_ptr<gfx::IGraphicsDevice> graphicsDevice);
      void initAnimationDataLayout(std::shared_ptr<gfx::IGraphicsDevice> graphicsDevice);

      std::unordered_map<LayoutHandle, std::unique_ptr<Layout>> layoutCache;
   };
}
