#pragma once

namespace ed {

class ControlBase {
public:
  ControlBase() = default;
  ControlBase(const ControlBase&) = default;
  ControlBase(ControlBase&&) = delete;
  auto operator=(const ControlBase&) -> ControlBase& = default;
  auto operator=(ControlBase&&) -> ControlBase& = delete;

  virtual ~ControlBase() = default;
  virtual void render() = 0;
  [[nodiscard]] virtual auto getValue() const -> std::any = 0;
};

}
