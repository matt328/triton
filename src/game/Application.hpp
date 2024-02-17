#pragma once

#include "core/Timer.hpp"
#include "game/Game.hpp"

namespace Triton::Game {
   class Application {
    public:
      Application(const int width, const int height, const std::string_view& windowTitle);
      ~Application();

      Application(const Application&) = delete;
      Application(Application&&) = delete;
      Application& operator=(const Application&) = delete;
      Application& operator=(Application&&) = delete;

      void update(const Core::Timer& timer);
      void render();

      void run(Core::Timer& timer);

    private:
      struct DestroyGlfwWindow {
         void operator()([[maybe_unused]] GLFWwindow* ptr) const {
         }
      };
      std::unique_ptr<GLFWwindow, DestroyGlfwWindow> window;
      std::unique_ptr<Game> game;
      bool running{};
      bool paused{};
      bool mouseCaptured{};
      bool fullscreen{};
      int prevXPos{}, prevYPos{}, prevWidth{}, prevHeight{};

      static void errorCallback(int code, const char* description);
      static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
      static void windowIconifiedCallback(GLFWwindow* window, int iconified);
      static void windowCloseCallback(GLFWwindow* window);
      static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
      static void cursorPosCallback(GLFWwindow* window, double xpos, double ypos);
      static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
   };
}
