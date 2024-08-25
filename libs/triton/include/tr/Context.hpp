#pragma once

#include "cm/Inputs.hpp"

namespace tr::ctx {

   class GameplayFacade;

   class Context {
    public:
      explicit Context(void* nativeWindow, bool guiEnabled = false, bool validationEnabled = false);
      ~Context();

      Context(const Context&) = delete;
      Context(Context&&) = delete;
      Context& operator=(const Context&) = delete;
      Context& operator=(Context&&) = delete;

      void start(const std::function<void()>& pollFn) const;

      void keyCallback(cm::Key key, cm::ButtonState buttonState) const;
      void cursorPosCallback(double xpos, double ypos) const;
      void mouseButtonCallback(int button, int action, int mods) const;
      void setMouseState(bool captured) const;
      void pause(bool paused) const;
      void hostWindowClosed() const;

      [[nodiscard]] GameplayFacade& getGameplayFacade() const;

      void setWireframe(bool wireframeEnabled) const;

    private:
      class Impl;
      std::unique_ptr<Impl> impl;
   };
}