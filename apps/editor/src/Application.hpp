#pragma once

#include "data/DataFacade.hpp"

/*
   Facade and context both need to be pImpls bc they're both separate entry classes each used by
   Editor and Game
*/

namespace tr::ctx {
   class Context;
   class GameplayFacade;
}

namespace ed {

   namespace ui {
      class Manager;
   }

   namespace data {
      class DataFacade;
   }

   class Application {
    public:
      Application(int width, int height, const std::string_view& windowTitle);
      ~Application();

      Application(const Application&) = delete;
      Application(Application&&) = delete;
      Application& operator=(const Application&) = delete;
      Application& operator=(Application&&) = delete;

      void run() const;

    private:
      struct DestroyGlfwWindow {
         void operator()([[maybe_unused]] GLFWwindow* ptr) const {
         }
      };

      struct DestroyContext {
         void operator()([[maybe_unused]] tr::ctx::Context* ptr) const {
         }
      };

      glm::vec4 color{};
      bool active = true;

      std::unique_ptr<GLFWwindow, DestroyGlfwWindow> window;
      std::unique_ptr<tr::ctx::Context> context;
      bool running{};
      bool paused{};
      bool mouseCaptured{};
      bool fullscreen{};
      int prevXPos{}, prevYPos{}, prevWidth{}, prevHeight{};

      std::unique_ptr<data::DataFacade> dataFacade;

      std::unique_ptr<ui::Manager> manager;

      void renderEntityEditor(tr::ctx::GameplayFacade& facade);
      void renderDockSpace();
      void renderMenuBar();

      static void toggleFullscreen(Application& app);

      static void errorCallback(int code, const char* description);
      static void windowIconifiedCallback(GLFWwindow* window, int iconified);
      static void windowCloseCallback(GLFWwindow* window);
      static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
      static void cursorPosCallback(GLFWwindow* window, double xpos, double ypos);
      static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
   };
}
