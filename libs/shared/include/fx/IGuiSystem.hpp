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

  // TODO(matt): this shouldn't be here.
  /*
    Probably split this thing into 2 pieces, have the renderCallback in one
    and the render function in another, and only expose the setRenderCallback part to the
    application
  */
  virtual auto render(vk::raii::CommandBuffer& commandBuffer,
                      const vk::ImageView& swapchainImageView,
                      const vk::Extent2D& swapchainExtent) -> void = 0;
};
}
