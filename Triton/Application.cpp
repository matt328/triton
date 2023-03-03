#include "Application.h"
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
}

Application::~Application() {
   glfwTerminate();
}

void Application::run() const {
   glfwSetKeyCallback(window.get(), keyCallback);
   while (!glfwWindowShouldClose(window.get())) {
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
