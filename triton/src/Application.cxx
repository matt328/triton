#include "Application.hpp"
#include "events/ApplicationEvent.hpp"
#include "events/KeyEvent.hpp"
#include "Logger.hpp"
#include "Events.hpp"
#include "actions/KeyMap.hpp"

namespace Triton {

   Application::Application(const int width, const int height, const std::string_view& windowTitle) {
      glfwInit();
      glfwSetErrorCallback(errorCallback);
      glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
      glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

      window.reset(glfwCreateWindow(width, height, windowTitle.data(), nullptr, nullptr));

      glfwSetWindowUserPointer(window.get(), this);
      glfwSetFramebufferSizeCallback(window.get(), framebufferResizeCallback);
   }

   Application::~Application() {
      glfwTerminate();
   };

   void Application::fireEvent(Events::Event& event) const {
      for (auto& fn : this->eventCallbackFnList) {
         fn(event);
      }
   }

   void Application::run() const {
      glfwSetKeyCallback(window.get(),
                         [](GLFWwindow* window,
                            const int key,
                            [[maybe_unused]] int scancode,
                            const int ation,
                            [[maybe_unused]] int mods) {
                            const auto app = static_cast<Application*>(glfwGetWindowUserPointer(window));
                            const auto mappedKey = Actions::keyMap[key];
                            switch (ation) {
                               case GLFW_PRESS: {
                                  Events::KeyPressedEvent event{mappedKey};
                                  app->fireEvent(event);
                                  break;
                               }
                               case GLFW_RELEASE: {
                                  Events::KeyReleasedEvent event{mappedKey};
                                  app->fireEvent(event);
                                  break;
                               }
                               case GLFW_REPEAT: {
                                  Events::KeyPressedEvent event{mappedKey, true};
                                  app->fireEvent(event);
                                  break;
                               }
                               default: {
                                  break;
                               }
                            }
                         });

      glfwSetCharCallback(window.get(), [](GLFWwindow* window, const unsigned int keyCode) {
         const auto app = static_cast<Application*>(glfwGetWindowUserPointer(window));
         const auto mappedKey = Actions::keyMap[static_cast<int>(keyCode)];
         Events::KeyTypedEvent event{mappedKey};
         app->fireEvent(event);
      });

      glfwSetWindowCloseCallback(window.get(), [](GLFWwindow* window) {
         const auto app = static_cast<Application*>(glfwGetWindowUserPointer(window));
         auto event = Events::WindowCloseEvent{};
         app->fireEvent(event);
         app->running = false;
      });

      double previousInstant = glfwGetTime();
      constexpr double maxFrameTime = 0.16667f;
      double accumulatedTime = 0.f;
      constexpr double fixedTimeStep = 1.f / 240.f;
      double currentInstant = glfwGetTime();

      while (running) {
         currentInstant = glfwGetTime();

         auto elapsed = currentInstant - previousInstant;

         if (elapsed > maxFrameTime) {
            elapsed = maxFrameTime;
         }
         accumulatedTime += elapsed;

         while (accumulatedTime >= fixedTimeStep) {
            {
               ZoneNamedN(update, "Update", true);
               auto event = Events::FixedUpdateEvent{};
               fireEvent(event);
            }
            accumulatedTime -= fixedTimeStep;
         }


         {
            const auto blendingFactor = accumulatedTime / fixedTimeStep;
            ZoneNamedN(blendState, "Blend State", true);
            auto event = Events::UpdateEvent{blendingFactor};
            fireEvent(event);
         }

         {
            ZoneNamedN(render, "Render", true);
            auto event = Events::RenderEvent{};
            fireEvent(event);
         }

         FrameMark;

         previousInstant = currentInstant;

         glfwPollEvents();
      }
      auto event = Events::ShutdownEvent{};
      fireEvent(event);
   }

   size_t Application::addEventCallbackFn(std::function<void(Events::Event&)> fn) {
      const auto position = eventCallbackFnList.size();
      eventCallbackFnList.push_back(fn);
      return position;
   }

   void Application::framebufferResizeCallback(GLFWwindow* window,
                                               const int width,
                                               const int height) {
      const auto app = static_cast<Application*>(glfwGetWindowUserPointer(window));
      app->context->windowResized(height, width);
   }

   void Application::errorCallback(int code, const char* description) {
      Log::error << "GLFW Error. Code: " << code << ", description: " << description << std::endl;
      throw std::runtime_error("GLFW Error. See log output for details");
   }
}
