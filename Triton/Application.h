#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <memory>

class RenderDevice;
class Instance;

struct DestroyGlfwWindow {
   void operator()(GLFWwindow* ptr) const {
      glfwDestroyWindow(ptr);
   }
};

class Application {
 public:
   Application();
   ~Application();

   void run() const;

 protected:
   std::unique_ptr<GLFWwindow, DestroyGlfwWindow> window = nullptr;
   std::unique_ptr<Instance> instance;
   std::unique_ptr<RenderDevice> renderDevice;

   void keyCallbackInt(int key, int scancode, int action, int mods) const;

   static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
   static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
};
