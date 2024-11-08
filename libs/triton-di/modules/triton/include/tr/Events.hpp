#pragma once

#include "Inputs.hpp"
#include "Actions.hpp"

namespace tr {

   struct WindowIconified {
      int iconified;
   };

   struct WindowClosed {};

   struct KeyEvent {
      tr::Key key;
      tr::ButtonState buttonState;
   };

   struct MouseMoved {
      double x;
      double y;
   };

   struct Fullscreen {
      bool isFullscreen;
   };

   struct MouseCaptured {
      bool isMouseCaptured;
   };

   struct MouseButton {
      int button;
      int action;
      int mods;
   };

   struct PlayerMoved {
      int playerId;
      float x, y;
   };

   struct PlayerScored {
      int playerId;
      int score;
   };

   using EventVariant = std::variant<WindowIconified,
                                     WindowClosed,
                                     KeyEvent,
                                     MouseMoved,
                                     MouseButton,
                                     Fullscreen,
                                     MouseCaptured,
                                     Action,
                                     PlayerMoved,
                                     PlayerScored>;

}