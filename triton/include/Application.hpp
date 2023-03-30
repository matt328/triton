#pragma once

class RenderDevice;
class Instance;
class Game;

struct DestroyGlfwWindow {
   void operator()(GLFWwindow* ptr) const {
      glfwDestroyWindow(ptr);
   }
};

class Application {
 public:
   Application();
   ~Application();

   Application(const Application&) = delete;
   Application(Application&&) = delete;
   Application& operator=(const Application&) = delete;
   Application& operator=(Application&&) = delete;

   void run() const;

 private:
   std::unique_ptr<GLFWwindow, DestroyGlfwWindow> window = nullptr;
   std::unique_ptr<Instance> instance;
   std::unique_ptr<RenderDevice> renderDevice;
   std::unique_ptr<Game> game;

   inline static constexpr double FRAME_TIME = 1.f / 60.f;

   void keyCallbackInt(int key, int scancode, int action, int mods) const;

   static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
   static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
};
