#include "Application.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "Instance.h"
#include "Log.h"
#include "RenderDevice.h"

Application::Application() {

   glfwInit();
   glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
   glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

   window = std::make_unique<GLFWwindow*>(glfwCreateWindow(1366, 768, "Vulkan", nullptr, nullptr));

   instance = std::make_unique<Instance>(window, true, 1366, 768);

   glfwSetWindowUserPointer(*window, this);
   glfwSetFramebufferSizeCallback(*window, framebufferResizeCallback);

   renderDevice = std::make_unique<RenderDevice>(instance);
}

Application::~Application() {
   glfwDestroyWindow(*window);
   glfwTerminate();
}

void Application::run() const {

   glfwSetKeyCallback(*window, keyCallback);

   while (!glfwWindowShouldClose(*window)) {
      glfwPollEvents();
   }
}

void Application::keyCallbackInt(const int key, int scancode, const int action, int mods) const {
   if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
      glfwSetWindowShouldClose(*window, GL_TRUE);
   }
}

void Application::framebufferResizeCallback(GLFWwindow* window, const int width, const int height) {
   const auto app = static_cast<Application*>(glfwGetWindowUserPointer(window));
   app->instance->resizeWindow(height, width);
}

void Application::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
   const auto app = static_cast<Application*>(glfwGetWindowUserPointer(window));
   app->keyCallbackInt(key, scancode, action, mods);
}
