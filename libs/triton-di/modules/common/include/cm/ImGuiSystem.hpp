#pragma once

#include "tr/IGuiSystem.hpp"

#include <gfx/QueueTypes.hpp>
#include <vk/Device.hpp>
#include <vk/VkResourceManager.hpp>

namespace tr::gfx {
   class Swapchain;
   class Instance;
   class IGraphicsDevice;
}

namespace tr {
   class IWindow;
}

namespace tr::cm {
   class ImGuiSystem final : public IGuiSystem {
    public:
      ImGuiSystem(const std::shared_ptr<IWindow>& window,
                  const std::shared_ptr<gfx::Instance>& instance,
                  const std::shared_ptr<gfx::Device>& device,
                  const std::shared_ptr<gfx::PhysicalDevice>& physicalDevice,
                  const std::shared_ptr<gfx::queue::Graphics>& graphicsQueue,
                  const std::shared_ptr<gfx::Swapchain>& swapchain,
                  std::shared_ptr<gfx::VkResourceManager> newResourceManager);
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
      std::shared_ptr<gfx::VkResourceManager> resourceManager;

      std::unique_ptr<vk::raii::DescriptorPool> descriptorPool;
      std::function<void()> renderFn;
   };
}