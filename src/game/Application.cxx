#include "Application.hpp"
#include <GLFW/glfw3.h>

namespace Triton::Game {

   constexpr auto MinHeight = 300;
   constexpr auto MinWidth = 200;
   constexpr auto SleepMillis = 100;

   Application::Application(const int width, const int height, const std::string_view& windowTitle)
       : window(nullptr), running(true) {
      glfwInit();
      glfwSetErrorCallback(errorCallback);
      glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
      glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

      window.reset(glfwCreateWindow(width, height, windowTitle.data(), nullptr, nullptr));

      glfwSetWindowSizeLimits(window.get(), MinHeight, MinWidth, GLFW_DONT_CARE, GLFW_DONT_CARE);

      if (glfwRawMouseMotionSupported())
         glfwSetInputMode(window.get(), GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);

      glfwSetWindowUserPointer(window.get(), this);
      glfwSetFramebufferSizeCallback(window.get(), framebufferResizeCallback);
      glfwSetWindowCloseCallback(window.get(), windowCloseCallback);
      glfwSetKeyCallback(window.get(), keyCallback);
      glfwSetCursorPosCallback(window.get(), cursorPosCallback);
      glfwSetMouseButtonCallback(window.get(), mouseButtonCallback);
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
            // normally sleep is bad, but this is just a temporary way to not hammer the cpu while
            // the application is minimized. Eventually, we'll probaby ask the game to pause itself,
            // and it can decide what that means.
            std::this_thread::sleep_for(std::chrono::milliseconds(SleepMillis));
            continue;
         }
         game->beginFrame();
         timer.tick([&]() { game->fixedUpdate(timer); });
         game->update();
         FrameMark;
      }
      game->waitIdle();
   }

   // Callbacks
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

   void Application::windowCloseCallback(GLFWwindow* window) {
      const auto app = static_cast<Application*>(glfwGetWindowUserPointer(window));
      app->running = false;
   }

   void Application::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
      const auto app = static_cast<Application*>(glfwGetWindowUserPointer(window));
      if (key == GLFW_KEY_ENTER && mods == GLFW_MOD_ALT && action == GLFW_RELEASE) {
         if (app->fullscreen) {
            glfwSetWindowMonitor(window,
                                 nullptr,
                                 app->prevXPos,
                                 app->prevYPos,
                                 app->prevWidth,
                                 app->prevHeight,
                                 0);
            app->fullscreen = !app->fullscreen;
         } else {
            const auto currentMonitor = glfwGetPrimaryMonitor();
            const auto mode = glfwGetVideoMode(currentMonitor);
            glfwGetWindowPos(window, &app->prevXPos, &app->prevYPos);
            glfwGetWindowSize(window, &app->prevWidth, &app->prevHeight);
            glfwSetWindowMonitor(window,
                                 currentMonitor,
                                 0,
                                 0,
                                 mode->width,
                                 mode->height,
                                 mode->refreshRate);
            app->fullscreen = !app->fullscreen;
         }
      } else {
         app->game->keyCallback(key, scancode, action, mods);
      }
   }

   void Application::cursorPosCallback(GLFWwindow* window, double xpos, double ypos) {
      const auto app = static_cast<Application*>(glfwGetWindowUserPointer(window));
      if (app->mouseCaptured) {
         app->game->cursorPosCallback(xpos, ypos);
      }
   }

   void Application::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
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
   }
}