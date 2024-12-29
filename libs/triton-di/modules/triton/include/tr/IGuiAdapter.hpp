#pragma once

namespace tr {
class IGuiAdapter {
public:
  IGuiAdapter() = default;
  virtual ~IGuiAdapter() = default;

  IGuiAdapter(const IGuiAdapter&) = default;
  IGuiAdapter(IGuiAdapter&&) = delete;
  auto operator=(const IGuiAdapter&) -> IGuiAdapter& = default;
  auto operator=(IGuiAdapter&&) -> IGuiAdapter& = delete;

  virtual auto needsKeyboard() -> bool = 0;
  virtual auto needsMouse() -> bool = 0;
  virtual auto disableMouse() -> void = 0;
  virtual auto enableMouse() -> void = 0;
};
}
