#include "Application.hpp"

#include "game/actions/KeyMap.hpp"
#include "game/events/KeyEvent.hpp"

namespace Triton::Game {

   Application::Application(const int width, const int height, const std::string_view& windowTitle)
       : window(nullptr), eventCallbackFnList({}), running(true) {
      glfwInit();
      glfwSetErrorCallback(errorCallback);
      glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
      glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

      window.reset(glfwCreateWindow(width, height, windowTitle.data(), nullptr, nullptr));

      glfwSetWindowUserPointer(window.get(), this);
      glfwSetFramebufferSizeCallback(window.get(), framebufferResizeCallback);

      glfwSetKeyCallback(window.get(),
                         [](GLFWwindow* window,
                            const int key,
                            [[maybe_unused]] int scancode,
                            int action,
                            [[maybe_unused]] int mods) {
                            const auto game =
                                static_cast<Application*>(glfwGetWindowUserPointer(window));
                            const auto mappedKey = Actions::keyMap[key];
                            switch (action) {
                               case GLFW_PRESS: {
                                  Events::KeyPressedEvent event{mappedKey};
                                  game->fireEvent(event);
                                  break;
                               }
                               case GLFW_RELEASE: {
                                  Events::KeyReleasedEvent event{mappedKey};
                                  game->fireEvent(event);
                                  break;
                               }
                               case GLFW_REPEAT: {
                                  Events::KeyPressedEvent event{mappedKey, true};
                                  game->fireEvent(event);
                                  break;
                               }
                               default: {
                                  break;
                               }
                            }
                         });

      glfwSetCharCallback(window.get(), [](GLFWwindow* window, const unsigned int keyCode) {
         const auto game = static_cast<Application*>(glfwGetWindowUserPointer(window));
         const auto mappedKey = Actions::keyMap[static_cast<int>(keyCode)];
         Events::KeyTypedEvent event{mappedKey};
         game->fireEvent(event);
      });

      glfwSetWindowCloseCallback(window.get(), [](GLFWwindow* window) {
         const auto game = static_cast<Application*>(glfwGetWindowUserPointer(window));
         game->running = false;
      });

      game = std::make_unique<Game>(window.get());
   }

   Application::~Application() {
      glfwTerminate();
   }

   void Application::run(Core::Timer& timer) {
      while (running) {
         timer.tick([&]() { game->update(timer); });
         game->render();
      }
      game->waitIdle();
   }

   size_t Application::addEventCallbackFn(std::function<void(Events::Event&)> fn) {
      const auto position = eventCallbackFnList.size();
      eventCallbackFnList.push_back(fn);
      return position;
   }

   void Application::fireEvent(Events::Event& event) const {
      for (auto& fn : this->eventCallbackFnList) {
         fn(event);
      }
   }

   void Application::resize([[maybe_unused]] const int width, [[maybe_unused]] const int height) {
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
}