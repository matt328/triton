#include "Application.hpp"
#include "Game.hpp"
#include "graphics/Instance.hpp"
#include "graphics/RenderDevice.hpp"
#include <client/TracyProfiler.hpp>

const auto width = 1366;
const auto height = 768;

Application::Application() {
   glfwInit();
   glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
   glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

   window.reset(glfwCreateWindow(width, height, "Vulkan", nullptr, nullptr));

   instance = std::make_unique<Instance>(window.get(), true, width, height);

   glfwSetWindowUserPointer(window.get(), this);
   glfwSetFramebufferSizeCallback(window.get(), framebufferResizeCallback);

   renderDevice = std::make_unique<RenderDevice>(*instance.get());

   game = std::make_unique<Game>(*renderDevice);
}

Application::~Application() {
   glfwTerminate();
}

void Application::run() const {
   glfwSetKeyCallback(window.get(), keyCallback);

   double t = 0.0;
   constexpr auto frameTimeThreshold = 0.25f;

   double currentTime = glfwGetTime();
   double accumulator = 0.f;

   while (!glfwWindowShouldClose(window.get())) {
      constexpr double dt = 0.01f;
      const double newTime = glfwGetTime();
      double frameTime = newTime - currentTime;
      if (frameTime > frameTimeThreshold) {
         frameTime = frameTimeThreshold;
      }

      currentTime = newTime;

      accumulator += frameTime;

      while (accumulator >= dt) {
         t += dt;
         accumulator -= dt;
      }

      const double alpha = accumulator / dt;

      {
         ZoneNamedN(blendState, "Blend State", true);
         game->blendState(alpha);
      }

      {
         ZoneNamedN(update, "Update", true);
         game->update(t, dt);
      }

      {
         ZoneNamedN(render, "Render", true);
         renderDevice->render();
      }

      FrameMark;

      glfwPollEvents();
   }
   renderDevice->waitIdle();
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
