#pragma once

namespace tr::gfx {
   class IGraphicsDevice;
}

namespace tr::gfx::sb {

   class Layout;

   enum class LayoutHandle : uint32_t {
      Invalid = 0,
      PerFrame = 1,
      Bindless = 2,
      ObjectData = 3,
      AnimationData = 4,
   };

   struct Config {
      bool useDescriptorBuffers;
      uint16_t maxTextures;
   };

   class LayoutFactory {

    public:
      explicit LayoutFactory(const std::shared_ptr<gfx::IGraphicsDevice>& graphicsDevice,
                             Config config);
      ~LayoutFactory();

      LayoutFactory(const LayoutFactory&) = delete;
      auto operator=(const LayoutFactory&) -> LayoutFactory& = delete;

      LayoutFactory(LayoutFactory&&) = delete;
      auto operator=(LayoutFactory&&) -> LayoutFactory& = delete;

      [[nodiscard]] auto getLayout(const LayoutHandle handle) const -> const Layout& {
         return *layoutCache.at(handle);
      }

      [[nodiscard]] auto getVkLayout(LayoutHandle handle) const -> const vk::DescriptorSetLayout&;

    private:
      Config config;

      void initBindlessLayout(std::shared_ptr<gfx::IGraphicsDevice> graphicsDevice);
      void initPerFrameLayout(std::shared_ptr<gfx::IGraphicsDevice> graphicsDevice);
      void initObjectDataLayout(std::shared_ptr<gfx::IGraphicsDevice> graphicsDevice);
      void initAnimationDataLayout(std::shared_ptr<gfx::IGraphicsDevice> graphicsDevice);

      std::unordered_map<LayoutHandle, std::unique_ptr<Layout>> layoutCache;
   };
}