#pragma once

namespace tr::gfx::ds {

   class Layout;

   enum class LayoutHandle : uint32_t {
      Invalid = 0,
      PerFrame = 1,
      Bindless = 2,
      ObjectData = 3,
   };

   class LayoutFactory {
    public:
      LayoutFactory(const vk::raii::Device& device);
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
      void initBindlessLayout(const vk::raii::Device& device);
      void initPerFrameLayout(const vk::raii::Device& device);
      void initObjectDataLayout(const vk::raii::Device& device);

      std::unordered_map<LayoutHandle, std::unique_ptr<Layout>> layoutCache;
   };
}