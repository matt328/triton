#pragma once

#include <vulkan/vulkan.hpp>

namespace tr {

class IWindow {
public:
  IWindow() = default;
  virtual ~IWindow() = default;

  IWindow(const IWindow&) = default;
  IWindow(IWindow&&) = delete;
  auto operator=(const IWindow&) -> IWindow& = default;
  auto operator=(IWindow&&) -> IWindow& = delete;

  virtual void pollEvents() = 0;
  virtual void setVulkanVersion(std::string_view version) = 0;
  virtual auto createVulkanSurface(const vk::Instance& instance, VkSurfaceKHR* outSurface) const
      -> void = 0;
  [[nodiscard]] virtual auto getFramebufferSize() const -> glm::ivec2 = 0;
  [[nodiscard]] virtual auto shouldClose() const -> bool = 0;
  [[nodiscard]] virtual auto getNativeWindow() const -> void* = 0;
};

}
