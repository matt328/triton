#pragma once

namespace tr {
class IGuiSystem {
public:
  IGuiSystem() = default;
  virtual ~IGuiSystem() = default;

  IGuiSystem(const IGuiSystem&) = default;
  IGuiSystem(IGuiSystem&&) = delete;
  auto operator=(const IGuiSystem&) -> IGuiSystem& = default;
  auto operator=(IGuiSystem&&) -> IGuiSystem& = delete;

  virtual auto setRenderCallback(std::function<void(void)> newRenderFn) -> void = 0;
  virtual auto render(vk::raii::CommandBuffer& commandBuffer,
                      const vk::ImageView& swapchainImageView,
                      const vk::Extent2D& swapchainExtent) -> void = 0;
};
}
