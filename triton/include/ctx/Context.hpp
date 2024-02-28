#pragma once

#include "util/Timer.hpp"

namespace tr::gp {
   class GameplaySystem;
}

namespace tr::gfx {
   class Renderer;
}

namespace tr::ctx {

   class GameplayFacade;

   class Context {
    public:
      Context(void* nativeWindow, bool guiEnabled = false); // TODO: revisit this at some point
      ~Context();

      Context(const Context&) = delete;
      Context(Context&&) = delete;
      Context& operator=(const Context&) = delete;
      Context& operator=(Context&&) = delete;

      void start(std::function<void()> pollFn);

      void keyCallback(int key, int scancode, int action, int mods);
      void cursorPosCallback(double xpos, double ypos);
      void mouseButtonCallback(int button, int action, int mods);
      void setMouseState(bool captured);
      void pause(bool paused);
      void hostWindowClosed();

      [[nodiscard]] GameplayFacade& getGameplayFacade() const {
         return *gameplayFacade;
      }

    private:
      bool running{}, paused{};
      tr::util::Timer timer;

      std::unique_ptr<GameplayFacade> gameplayFacade;

      std::unique_ptr<tr::gp::GameplaySystem> gameplaySystem;
      std::unique_ptr<tr::gfx::Renderer> renderer;
   };
}