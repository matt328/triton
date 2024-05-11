#pragma once

namespace tr::gfx {
   class GraphicsDevice;
}

namespace tr::gfx::ds {

   class Layout;

   enum class LayoutHandle : uint32_t {
      Invalid = 0,
      PerFrame = 1,
      Bindless = 2,
      ObjectData = 3,
      AnimationData = 4,
   };

   class LayoutFactory {
    public:
      LayoutFactory(const GraphicsDevice& device, const bool useDescriptorBuffers = false);
      ~LayoutFactory();

      LayoutFactory(const LayoutFactory&) = delete;
      LayoutFactory& operator=(const LayoutFactory&) = delete;

      LayoutFactory(LayoutFactory&&) = delete;
      LayoutFactory& operator=(LayoutFactory&&) = delete;

      [[nodiscard]] const Layout& getLayout(LayoutHandle handle) const {
         return *layoutCache.at(handle);
      }

      [[nodiscard]] const vk::DescriptorSetLayout& getVkLayout(LayoutHandle handle) const;

    private:
      void initBindlessLayout(const GraphicsDevice& device);
      void initPerFrameLayout(const GraphicsDevice& device);
      void initObjectDataLayout(const GraphicsDevice& device);
      void initAnimationDataLayout(const GraphicsDevice& device);

      const bool useDescriptorBuffers{};
      std::unordered_map<LayoutHandle, std::unique_ptr<Layout>> layoutCache;
   };
}