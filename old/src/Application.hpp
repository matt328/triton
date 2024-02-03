#pragma once

#include "Renderer.hpp"
#include "ResourceFactory.hpp"
#include "Events.hpp"

namespace Triton {

   class Application {
    public:
      Application(int width, int height, const std::string_view& windowTitle);
      ~Application();

      Application(const Application&) = delete;
      Application(Application&&) = delete;
      Application& operator=(const Application&) = delete;
      Application& operator=(Application&&) = delete;

      void run() const;

      ResourceFactory* getResourceFactory();

      size_t registerUpdate(std::function<void(void)> fn);
      size_t registerUpdateBlendState(std::function<void(double)> fn);
      size_t registerKeyHandler(std::function<void(int, int, int, int)> fn);

      void deregisterUpdate(size_t num);
      void deregisterUpdateBlendState(size_t num);
      void deregisterKeyHandler(size_t num);

      size_t addEventCallbackFn(std::function<void(Events::Event&)> fn);

      void fireEvent(Events::Event& event) const;

      [[nodiscard]] GLFWwindow* getWindow() const {
         return window.get();
      }

      static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
      static void errorCallback(int code, const char* description);

    private:
      struct DestroyGlfwWindow {
         void operator()([[maybe_unused]] GLFWwindow* ptr) const {
            // This only exists to trick unique_ptr into allowing me to forward declare the impl
         }
      };
      std::unique_ptr<GLFWwindow, DestroyGlfwWindow> window;
      std::shared_ptr<Renderer> context;

      std::vector<std::function<void(Events::Event&)>> eventCallbackFnList;

      inline static constexpr double FRAME_TIME = 1.f / 60.f;
      bool running = true;
   };
};
