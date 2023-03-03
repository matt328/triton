#include "Application.h"
#include "Game.h"
#include "Instance.h"
#include "RenderDevice.h"

Application::Application() {

   glfwInit();
   glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
   glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

   window.reset(glfwCreateWindow(1366, 768, "Vulkan", nullptr, nullptr));

   instance = std::make_unique<Instance>(window.get(), true, 1366, 768);

   glfwSetWindowUserPointer(window.get(), this);
   glfwSetFramebufferSizeCallback(window.get(), framebufferResizeCallback);

   renderDevice = std::make_unique<RenderDevice>(*instance.get());

   game = std::make_unique<Game>();
}

Application::~Application() {
   glfwTerminate();
}

void Application::run() const {
   glfwSetKeyCallback(window.get(), keyCallback);

   double t = 0.f;

   double currentTime = glfwGetTime();
   double accumulator = 0.f;

   while (!glfwWindowShouldClose(window.get())) {
      constexpr double dt = 0.01f;
      const double newTime = glfwGetTime();
      double frameTime = newTime - currentTime;
      if (frameTime > 0.25f) {
         frameTime = 0.25f;
      }
      currentTime = newTime;

      accumulator += frameTime;

      while (accumulator >= dt) {
         game->update(t, dt);
         t += dt;
         accumulator -= dt;
      }

      const double alpha = accumulator / dt;

      game->blendState(alpha);

      renderDevice->render(*game);

      glfwPollEvents();
   }
}

void Application::keyCallbackInt(const int key, int scancode, const int action, int mods) const {
   if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
      glfwSetWindowShouldClose(window.get(), GL_TRUE);
   }
}

void Application::framebufferResizeCallback(GLFWwindow* window, const int width, const int height) {
   const auto app = static_cast<Application*>(glfwGetWindowUserPointer(window));
   app->instance->resizeWindow(height, width);
}

void Application::keyCallback(
    GLFWwindow* window, const int key, const int scancode, const int action, const int mods) {
   const auto app = static_cast<Application*>(glfwGetWindowUserPointer(window));
   app->keyCallbackInt(key, scancode, action, mods);
}
