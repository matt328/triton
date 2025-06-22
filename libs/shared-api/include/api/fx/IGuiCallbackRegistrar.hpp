#pragma once

namespace tr {
class IGuiCallbackRegistrar {
public:
  IGuiCallbackRegistrar() = default;
  virtual ~IGuiCallbackRegistrar() = default;

  IGuiCallbackRegistrar(const IGuiCallbackRegistrar&) = default;
  IGuiCallbackRegistrar(IGuiCallbackRegistrar&&) = delete;
  auto operator=(const IGuiCallbackRegistrar&) -> IGuiCallbackRegistrar& = default;
  auto operator=(IGuiCallbackRegistrar&&) -> IGuiCallbackRegistrar& = delete;

  virtual auto setRenderCallback(std::function<void(void)> newRenderFn) -> void = 0;
  virtual auto render() -> void = 0;

  virtual auto setReadyCallback(std::function<void(void)> newReadyFn) -> void = 0;
  virtual auto ready() -> void = 0;
};
}
