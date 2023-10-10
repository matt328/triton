#include "Application.hpp"
#include "ApplicationEvent.h"
#include "Context.hpp"
#include "KeyEvent.h"
#include "ResourceFactory.hpp"
#include "Logger.hpp"
#include <GLFW/glfw3.h>
#include "Events.hpp"
#include "input/KeyMap.hpp"

class Application::ApplicationImpl {
 public:
   ApplicationImpl(const ApplicationImpl&) = delete;
   ApplicationImpl(ApplicationImpl&&) = delete;
   ApplicationImpl& operator=(const ApplicationImpl&) = delete;
   ApplicationImpl& operator=(ApplicationImpl&&) = delete;

   void setEventCallbackFn(std::function<void(Events::Event&)> fn) {
      this->eventCallbackFn = fn;
   }

   void registerRenderObjectProvider(std::function<std::vector<RenderObject>()> fn) {
      context->registerRenderObjectProvider(fn);
   }

   void registerPerFrameDataProvider(std::function<PerFrameData()> fn) {
      context->registerPerFrameDataProvider(fn);
   }

   static void framebufferResizeCallback(GLFWwindow* window, const int width, const int height) {
      const auto app = static_cast<ApplicationImpl*>(glfwGetWindowUserPointer(window));
      app->context->windowResized(height, width);
   }

   static void errorCallback(int code, const char* description) {
      Log::error << "GLFW Error. Code: " << code << ", description: " << description << std::endl;
      throw std::runtime_error("GLFW Error. See log output for details");
   }

   ApplicationImpl(int width, int height, const std::string_view& windowTitle) {
      glfwInit();
      glfwSetErrorCallback(errorCallback);
      glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
      glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

      window.reset(glfwCreateWindow(width, height, windowTitle.data(), nullptr, nullptr));

      glfwSetWindowUserPointer(window.get(), this);
      glfwSetFramebufferSizeCallback(window.get(), framebufferResizeCallback);

      context = std::make_shared<graphics::Context>(window.get());
   }

   IResourceFactory* getResourceFactory() {
      return context.get();
   }

   ~ApplicationImpl() {
      glfwTerminate();
   }

   void run() const {
      glfwSetKeyCallback(window.get(),
                         [](GLFWwindow* window,
                            int key,
                            [[maybe_unused]] int scancode,
                            int ation,
                            [[maybe_unused]] int mods) {
                            auto app =
                                static_cast<ApplicationImpl*>(glfwGetWindowUserPointer(window));
                            const auto mappedKey = Input::keyMap[key];
                            switch (ation) {
                               case GLFW_PRESS: {
                                  Events::KeyPressedEvent event{mappedKey};
                                  app->eventCallbackFn(event);
                                  break;
                               }
                               case GLFW_RELEASE: {
                                  Events::KeyReleasedEvent event{mappedKey};
                                  app->eventCallbackFn(event);
                                  break;
                               }
                               case GLFW_REPEAT: {
                                  Events::KeyPressedEvent event{mappedKey, true};
                                  app->eventCallbackFn(event);
                                  break;
                               }
                            }
                         });

      glfwSetCharCallback(window.get(), [](GLFWwindow* window, unsigned int keyCode) {
         auto app = static_cast<ApplicationImpl*>(glfwGetWindowUserPointer(window));
         // Need to check this if we start collectingb char input
         const auto mappedKey = Input::keyMap[(int)keyCode];
         Events::KeyTypedEvent event{mappedKey};
         app->eventCallbackFn(event);
      });

      glfwSetWindowCloseCallback(window.get(), [](GLFWwindow* window) {
         auto app = static_cast<ApplicationImpl*>(glfwGetWindowUserPointer(window));
         auto event = Events::WindowCloseEvent{};
         app->eventCallbackFn(event);
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
               eventCallbackFn(event);
            }
            accumulatedTime -= fixedTimeStep;
         }

         auto blendingFactor = accumulatedTime / fixedTimeStep;

         {
            ZoneNamedN(blendState, "Blend State", true);
            auto event = Events::UpdateEvent{blendingFactor};
            eventCallbackFn(event);
         }

         {
            ZoneNamedN(render, "Render", true);
            auto event = Events::RenderEvent{};
            eventCallbackFn(event);
            context->render();
         }

         FrameMark;

         previousInstant = currentInstant;

         glfwPollEvents();
      }
      auto event = Events::ShutdownEvent{};
      eventCallbackFn(event);
      context->waitIdle();
   }

 private:
   struct DestroyGlfwWindow {
      void operator()([[maybe_unused]] GLFWwindow* ptr) const {
         // This only exists to trick unique_ptr into allowing me to forward declare the impl
      }
   };
   std::unique_ptr<GLFWwindow, DestroyGlfwWindow> window;
   std::shared_ptr<graphics::Context> context;
   std::vector<std::function<void(void)>> updates;
   std::vector<std::function<void(double)>> blendUpdates;
   std::vector<std::function<void(int, int, int, int)>> keyHandlers;

   std::function<void(Events::Event&)> eventCallbackFn;

   inline static constexpr double FRAME_TIME = 1.f / 60.f;
   bool running = true;
};

Application::Application(int width, int height, const std::string_view& windowTitle) {
   impl = std::make_unique<ApplicationImpl>(width, height, windowTitle);
}

Application::~Application() = default;

void Application::run() const {
   impl->run();
}

IResourceFactory* Application::getResourceFactory() {
   return impl->getResourceFactory();
}

void Application::setEventCallbackFn(std::function<void(Events::Event&)> fn) {
   impl->setEventCallbackFn(fn);
}

void Application::registerRenderObjectProvider(std::function<std::vector<RenderObject>()> fn) {
   impl->registerRenderObjectProvider(fn);
}

void Application::registerPerFrameDataProvider(std::function<PerFrameData()> fn) {
   impl->registerPerFrameDataProvider(fn);
}