#pragma once

#include "cm/Inputs.hpp"
#include "cm/Timer.hpp"

namespace tr::gp {
   class GameplaySystem;
}

namespace tr::gfx {
   class RenderContext;
}

namespace tr::ctx {

   class GameplayFacade;

   class Context {
    public:
      Context(void* nativeWindow,
              bool guiEnabled = false,
              bool debugEnabled = false); // TODO: revisit this at some point
      ~Context();

      Context(const Context&) = delete;
      Context(Context&&) = delete;
      Context& operator=(const Context&) = delete;
      Context& operator=(Context&&) = delete;

      void start(std::function<void()> pollFn);

      void keyCallback(cm::Key key, cm::ButtonState buttonState);
      void cursorPosCallback(double xpos, double ypos);
      void mouseButtonCallback(int button, int action, int mods);
      void setMouseState(bool captured);
      void pause(bool paused);
      void hostWindowClosed();

      [[nodiscard]] GameplayFacade& getGameplayFacade() const {
         return *gameplayFacade;
      }

      void setWireframe(bool wireframeEnabled);

    private:
      bool running{}, paused{};
      tr::cm::Timer timer;

      std::unique_ptr<GameplayFacade> gameplayFacade;

      std::unique_ptr<tr::gp::GameplaySystem> gameplaySystem;
      std::unique_ptr<tr::gfx::RenderContext> renderContext;
   };
}