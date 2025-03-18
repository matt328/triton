#pragma once

#include "Inputs.hpp"
#include "Actions.hpp"

#include "GameplayEvents.hpp"
#include "tr/GameplayEvents.hpp"

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

struct SwapchainResized {
  uint32_t width;
  uint32_t height;
};

struct SwapchainCreated {
  uint32_t width;
  uint32_t height;
};

struct FrameEndEvent {
  vk::Fence fence;
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
                                  PlayerScored,
                                  EntityCreated,
                                  SwapchainResized,
                                  SwapchainCreated,
                                  FrameEndEvent>;

}
