#pragma once

#include <GLFW/glfw3.h>

#include "api/fx/IWindow.hpp"

namespace ed {
class Properties;
}

namespace tr {

class IEventQueue;
class IGuiAdapter;

struct WindowCreateInfo {
  int32_t height;
  int32_t width;
  std::string title;
};

class GlfwWindow : public IWindow {
public:
  explicit GlfwWindow(const WindowCreateInfo& createInfo,
                      std::shared_ptr<IEventQueue> newEventBus,
                      std::shared_ptr<IGuiAdapter> newGuiAdapter,
                      std::shared_ptr<ed::Properties> newProperties);
  ~GlfwWindow() override;

  GlfwWindow(const GlfwWindow&) = default;
  GlfwWindow(GlfwWindow&&) = delete;
  auto operator=(const GlfwWindow&) -> GlfwWindow& = default;
  auto operator=(GlfwWindow&&) -> GlfwWindow& = delete;

  void pollEvents() override;
  void setVulkanVersion(std::string_view version) override;
  auto createVulkanSurface(const vk::Instance& instance, VkSurfaceKHR* outSurface) const
      -> void override;
  [[nodiscard]] auto getFramebufferSize() const -> glm::ivec2 override;

  [[nodiscard]] auto shouldClose() const -> bool override;

  [[nodiscard]] auto getNativeWindow() const -> void* override;

private:
  std::shared_ptr<IEventQueue> eventBus;
  std::shared_ptr<IGuiAdapter> guiAdapter;
  std::shared_ptr<ed::Properties> properties;

  GLFWwindow* window;

  bool isFullscreen{};
  bool isMouseCaptured{};
  int prevXPos{}, prevYPos{}, prevWidth{}, prevHeight{};

  auto toggleFullscreen() -> void;

  static void errorCallback(int code, const char* description);
  static void windowIconifiedCallback(GLFWwindow* window, int iconified);
  static void windowCloseCallback(GLFWwindow* window);
  static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
  static void cursorPosCallback(GLFWwindow* window, double xpos, double ypos);
  static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
};

}
