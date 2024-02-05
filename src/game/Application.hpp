#pragma once

#include "core/Timer.hpp"
#include "game/events/Events.hpp"
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

      void resize(const int width, const int height);

      size_t addEventCallbackFn(std::function<void(Events::Event&)> fn);

      void run(Core::Timer& timer);

    private:
      struct DestroyGlfwWindow {
         void operator()([[maybe_unused]] GLFWwindow* ptr) const {
         }
      };
      std::unique_ptr<GLFWwindow, DestroyGlfwWindow> window;
      std::vector<std::function<void(Events::Event&)>> eventCallbackFnList;
      std::unique_ptr<Game> game;
      bool running;

      void fireEvent(Events::Event& event) const;

      static void errorCallback(int code, const char* description);
      static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
   };
}
