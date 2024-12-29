#pragma once

namespace tr {

class IWindow {
public:
  IWindow() = default;
  virtual ~IWindow() = default;

  IWindow(const IWindow&) = default;
  IWindow(IWindow&&) = delete;
  auto operator=(const IWindow&) -> IWindow& = default;
  auto operator=(IWindow&&) -> IWindow& = delete;

  virtual void pollEvents() = 0;

  virtual auto getNativeWindow() -> void* = 0;
};
}
