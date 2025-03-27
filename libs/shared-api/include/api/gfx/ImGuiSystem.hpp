#pragma once

#include "IGuiSystem.hpp"

#include "gfx/QueueTypes.hpp"
#include "vk/core/Device.hpp"
#include "VkResourceManager.hpp"

namespace tr {

class Swapchain;
class Instance;
class IWindow;
class IGuiCallbackRegistrar;

class ImGuiSystem final : public IGuiSystem {
public:
  ImGuiSystem(const std::shared_ptr<IWindow>& window,
              const std::shared_ptr<Instance>& instance,
              std::shared_ptr<Device> newDevice,
              const std::shared_ptr<PhysicalDevice>& physicalDevice,
              const std::shared_ptr<queue::Graphics>& graphicsQueue,
              const std::shared_ptr<Swapchain>& swapchain,
              std::shared_ptr<VkResourceManager> newResourceManager,
              std::shared_ptr<IGuiCallbackRegistrar> newGuiCallbackRegistrar);
  ~ImGuiSystem() override;

  ImGuiSystem(const ImGuiSystem&) = delete;
  ImGuiSystem(ImGuiSystem&&) = delete;
  auto operator=(const ImGuiSystem&) -> ImGuiSystem& = delete;
  auto operator=(ImGuiSystem&&) -> ImGuiSystem& = delete;

  auto render(vk::raii::CommandBuffer& commandBuffer,
              const vk::ImageView& swapchainImageView,
              const vk::Extent2D& swapchainExtent) -> void override;

private:
  std::shared_ptr<VkResourceManager> resourceManager;
  std::shared_ptr<Device> device;
  std::shared_ptr<IGuiCallbackRegistrar> guiCallbackRegistrar;

  std::unique_ptr<vk::raii::DescriptorPool> descriptorPool;
  std::function<void()> renderFn;
};
}
