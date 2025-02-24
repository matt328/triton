#pragma once

#include "tr/IGuiAdapter.hpp"
#include "tr/IWindow.hpp"
#include "tr/IEventBus.hpp"

namespace tr {

class Window : public tr::IWindow {
public:
  Window(std::shared_ptr<IEventBus> newEventBus,
         std::shared_ptr<IGuiAdapter> newGuiAdapter,
         const glm::ivec2& dimensions,
         const std::string& windowTitle);
  ~Window() override;

  Window(const Window&) = default;
  Window(Window&&) = delete;
  auto operator=(const Window&) -> Window& = delete;
  auto operator=(Window&&) -> Window& = delete;

  auto getNativeWindow() -> void* override;

private:
  GLFWwindow* window;
  std::shared_ptr<tr::IEventBus> eventBus;
  std::shared_ptr<tr::IGuiAdapter> guiAdapter;

  bool isFullscreen{};
  bool isMouseCaptured{};
  int prevXPos{}, prevYPos{}, prevWidth{}, prevHeight{};

  void toggleFullscreen();

  void pollEvents() override;

  void setVulkanVersion(std::string_view version) override;

  static void errorCallback(int code, const char* description);
  static void windowIconifiedCallback(GLFWwindow* window, int iconified);
  static void windowCloseCallback(GLFWwindow* window);
  static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
  static void cursorPosCallback(GLFWwindow* window, double xpos, double ypos);
  static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
};
}
