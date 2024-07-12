#pragma once

#include "cm/Inputs.hpp"

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

      [[nodiscard]] GameplayFacade& getGameplayFacade() const;

      void setWireframe(bool wireframeEnabled);

    private:
      class Impl;
      std::unique_ptr<Impl> impl;
   };
}