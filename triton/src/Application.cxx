#include "Application.hpp"
#include "Context.hpp"
#include "ResourceFactory.hpp"
#include "Logger.hpp"
#include <GLFW/glfw3.h>
#include "Events.hpp"

class Application::ApplicationImpl {
 public:
   ApplicationImpl(const ApplicationImpl&) = delete;
   ApplicationImpl(ApplicationImpl&&) = delete;
   ApplicationImpl& operator=(const ApplicationImpl&) = delete;
   ApplicationImpl& operator=(ApplicationImpl&&) = delete;

   void setEventCallbackFn(std::function<void(Events::Event&)> fn) {
      this->eventCallbackFn = fn;
   }

   void registerGame(std::shared_ptr<IGame> game) {
      this->game = game;
      this->context->registerGame(game);
   }

   void keyCallbackInt(const int key, int scancode, const int action, int mods) const {
      if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
         glfwSetWindowShouldClose(window.get(), GL_TRUE);
      }
      if (game != nullptr) {
         game->keyPressed(key, scancode, action, mods);
      } else {
         Log::warn << "Keys pressed before game was registered" << std::endl;
      }
   }

   static void framebufferResizeCallback(GLFWwindow* window, const int width, const int height) {
      const auto app = static_cast<ApplicationImpl*>(glfwGetWindowUserPointer(window));
      app->context->windowResized(height, width);
   }

   static void keyCallback(
       GLFWwindow* window, const int key, const int scancode, const int action, const int mods) {
      const auto app = static_cast<ApplicationImpl*>(glfwGetWindowUserPointer(window));
      app->keyCallbackInt(key, scancode, action, mods);
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
      if (game == nullptr) {
         throw std::runtime_error{"Game not registered, bailing out"};
      }
      glfwSetKeyCallback(window.get(), keyCallback);

      double previousInstant = glfwGetTime();
      constexpr double maxFrameTime = 0.16667f;
      double accumulatedTime = 0.f;
      constexpr double fixedTimeStep = 1.f / 240.f;
      double currentInstant = glfwGetTime();

      while (!glfwWindowShouldClose(window.get())) {
         currentInstant = glfwGetTime();

         auto elapsed = currentInstant - previousInstant;

         if (elapsed > maxFrameTime) {
            elapsed = maxFrameTime;
         }
         accumulatedTime += elapsed;

         while (accumulatedTime >= fixedTimeStep) {
            {
               ZoneNamedN(update, "Update", true);
               game->update();
            }
            accumulatedTime -= fixedTimeStep;
         }

         auto blendingFactor = accumulatedTime / fixedTimeStep;

         {
            ZoneNamedN(blendState, "Blend State", true);
            game->blendState(blendingFactor);
         }

         {
            ZoneNamedN(render, "Render", true);
            context->render();
         }

         FrameMark;

         previousInstant = currentInstant;

         glfwPollEvents();
      }
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
   std::shared_ptr<IGame> game;

   std::function<void(Events::Event&)> eventCallbackFn;

   inline static constexpr double FRAME_TIME = 1.f / 60.f;
};

Application::Application(int width, int height, const std::string_view& windowTitle) {
   impl = std::make_unique<ApplicationImpl>(width, height, windowTitle);
}

Application::~Application() = default;

void Application::run() const {
   impl->run();
}

void Application::registerGame(std::shared_ptr<IGame> game) {
   impl->registerGame(game);
}

IResourceFactory* Application::getResourceFactory() {
   return impl->getResourceFactory();
}

void Application::setEventCallbackFn(std::function<void(Events::Event&)> fn) {
   impl->setEventCallbackFn(fn);
}
