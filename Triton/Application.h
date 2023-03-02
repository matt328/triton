#pragma once

#include <memory>

struct GLFWwindow;
class RenderDevice;
class Instance;

class Application {
 public:
   Application();
   ~Application();

   void run() const;

 protected:
   std::unique_ptr<GLFWwindow*> window;
   std::unique_ptr<Instance> instance;
   std::unique_ptr<RenderDevice> renderDevice;

   void keyCallbackInt(int key, int scancode, int action, int mods) const;

   static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
   static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
};
