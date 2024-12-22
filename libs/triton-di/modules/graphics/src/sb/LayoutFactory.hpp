#pragma once

#include "gfx/IRenderContext.hpp"
#include "sb/ILayoutFactory.hpp"

namespace tr {
   class IGraphicsDevice;
}

namespace tr {

   class LayoutFactory : public ILayoutFactory {

    public:
      LayoutFactory(const std::shared_ptr<IGraphicsDevice>& graphicsDevice,
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

      void initBindlessLayout(std::shared_ptr<IGraphicsDevice> graphicsDevice);
      void initPerFrameLayout(std::shared_ptr<IGraphicsDevice> graphicsDevice);
      void initObjectDataLayout(std::shared_ptr<IGraphicsDevice> graphicsDevice);
      void initAnimationDataLayout(std::shared_ptr<IGraphicsDevice> graphicsDevice);

      std::unordered_map<LayoutHandle, std::unique_ptr<Layout>> layoutCache;
   };
}
