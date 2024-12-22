#pragma once

namespace tr {

   class Layout;

   enum class LayoutHandle : uint32_t {
      Invalid = 0,
      PerFrame = 1,
      Bindless = 2,
      ObjectData = 3,
      AnimationData = 4,
   };

   class ILayoutFactory {
    public:
      ILayoutFactory() = default;
      virtual ~ILayoutFactory() = default;

      ILayoutFactory(const ILayoutFactory&) = default;
      ILayoutFactory(ILayoutFactory&&) = delete;
      auto operator=(const ILayoutFactory&) -> ILayoutFactory& = default;
      auto operator=(ILayoutFactory&&) -> ILayoutFactory& = delete;

      [[nodiscard]] virtual auto getLayout(LayoutHandle handle) const -> const Layout& = 0;
      [[nodiscard]] virtual auto getVkLayout(LayoutHandle handle) const
          -> const vk::DescriptorSetLayout& = 0;
   };
}