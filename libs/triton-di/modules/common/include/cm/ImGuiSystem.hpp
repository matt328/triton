#pragma once

#include "tr/IGuiSystem.hpp"

namespace tr::gfx {
   class IGraphicsDevice;
}

namespace tr {
   class IWindow;
}

namespace tr::cm {
   class ImGuiSystem : public IGuiSystem {
    public:
      ImGuiSystem(const std::shared_ptr<gfx::IGraphicsDevice>& graphicsDevice,
                  const std::shared_ptr<IWindow>& window);
      ~ImGuiSystem() override;

      ImGuiSystem(const ImGuiSystem&) = delete;
      ImGuiSystem(ImGuiSystem&&) = delete;
      auto operator=(const ImGuiSystem&) -> ImGuiSystem& = delete;
      auto operator=(ImGuiSystem&&) -> ImGuiSystem& = delete;

      auto setRenderCallback(std::function<void(void)> newRenderFn) -> void override;
      auto render(const std::unique_ptr<vk::raii::CommandBuffer>& commandBuffer,
                  const vk::raii::ImageView& swapchainImageView,
                  const vk::Extent2D& swapchainExtent) -> void override;

    private:
      std::unique_ptr<vk::raii::DescriptorPool> descriptorPool;
      std::function<void(void)> renderFn;
   };
}