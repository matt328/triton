#pragma once

#include "cm/IWindow.hpp"
#include "cm/event/IEventBus.hpp"

namespace ed {

   class Window : public tr::cm::IWindow {
    public:
      Window(const glm::ivec2& dimensions,
             const std::string& windowTitle,
             const std::shared_ptr<tr::cm::evt::IEventBus>& eventBus);
      ~Window() override;

      Window(const Window&) = default;
      Window(Window&&) = delete;
      auto operator=(const Window&) -> Window& = delete;
      auto operator=(Window&&) -> Window& = delete;

      auto getNativeWindow() -> void* override;

    private:
      GLFWwindow* window;
      std::shared_ptr<tr::cm::evt::IEventBus> eventBus;

      bool isFullscreen{};
      bool isMouseCaptured{};
      int prevXPos{}, prevYPos{}, prevWidth{}, prevHeight{};

      void toggleFullscreen();

      void pollEvents() override;

      static void errorCallback(int code, const char* description);
      static void windowIconifiedCallback(GLFWwindow* window, int iconified);
      static void windowCloseCallback(GLFWwindow* window);
      static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
      static void cursorPosCallback(GLFWwindow* window, double xpos, double ypos);
      static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
   };
}