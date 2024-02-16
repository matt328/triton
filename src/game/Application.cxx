#include "Application.hpp"

namespace Triton::Game {

   Application::Application(const int width, const int height, const std::string_view& windowTitle)
       : window(nullptr), running(true) {
      glfwInit();
      glfwSetErrorCallback(errorCallback);
      glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
      glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

      window.reset(glfwCreateWindow(width, height, windowTitle.data(), nullptr, nullptr));

      if (glfwRawMouseMotionSupported())
         glfwSetInputMode(window.get(), GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);

      glfwSetWindowUserPointer(window.get(), this);
      glfwSetFramebufferSizeCallback(window.get(), framebufferResizeCallback);

      glfwSetWindowCloseCallback(window.get(), [](GLFWwindow* window) {
         const auto app = static_cast<Application*>(glfwGetWindowUserPointer(window));
         app->running = false;
      });

      glfwSetKeyCallback(window.get(),
                         [](GLFWwindow* window, int key, int scancode, int action, int mods) {
                            const auto app =
                                static_cast<Application*>(glfwGetWindowUserPointer(window));
                            app->game->keyCallback(key, scancode, action, mods);
                         });

      glfwSetCursorPosCallback(window.get(), [](GLFWwindow* window, double xpos, double ypos) {
         const auto app = static_cast<Application*>(glfwGetWindowUserPointer(window));
         if (app->mouseCaptured) {
            app->game->cursorPosCallback(xpos, ypos);
         }
      });

      glfwSetMouseButtonCallback(
          window.get(),
          [](GLFWwindow* window, int button, int action, int mods) {
             const auto app = static_cast<Application*>(glfwGetWindowUserPointer(window));
             if (button == GLFW_MOUSE_BUTTON_1 && action == GLFW_RELEASE) {
                if (!app->mouseCaptured) {
                   glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                } else {
                   glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
                }
                app->mouseCaptured = !app->mouseCaptured;
                app->game->setMouseState(app->mouseCaptured);
             }
             app->game->mouseButtonCallback(button, action, mods);
          });

      glfwSetWindowIconifyCallback(window.get(), windowIconifiedCallback);

      game = std::make_unique<Game>(window.get());
   }

   Application::~Application() {
      glfwTerminate();
   }

   void Application::run(Core::Timer& timer) {
      while (running) {
         glfwPollEvents();
         if (paused) {
            // avoid crushing the cpu
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
         }
         game->beginFrame();
         timer.tick([&]() { game->fixedUpdate(timer); });
         game->update();
         FrameMark;
      }
      game->waitIdle();
   }

   void Application::errorCallback(int code, const char* description) {
      Log::error << "GLFW Error. Code: " << code << ", description: " << description << std::endl;
      throw std::runtime_error("GLFW Error. See log output for details");
   }

   void Application::framebufferResizeCallback(GLFWwindow* window,
                                               const int width,
                                               const int height) {
      const auto app = static_cast<Application*>(glfwGetWindowUserPointer(window));
      app->game->resize(width, height);
   }

   void Application::windowIconifiedCallback(GLFWwindow* window, int iconified) {
      const auto app = static_cast<Application*>(glfwGetWindowUserPointer(window));
      // Just stop crashing for now.
      app->paused = iconified;
   }
}