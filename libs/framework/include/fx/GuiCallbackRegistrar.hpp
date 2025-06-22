#pragma once

#include "api/fx/IGuiCallbackRegistrar.hpp"

namespace tr {

class GuiCallBackRegistrar : public IGuiCallbackRegistrar {
public:
  GuiCallBackRegistrar() = default;
  ~GuiCallBackRegistrar() override = default;

  GuiCallBackRegistrar(const GuiCallBackRegistrar&) = default;
  GuiCallBackRegistrar(GuiCallBackRegistrar&&) = delete;
  auto operator=(const GuiCallBackRegistrar&) -> GuiCallBackRegistrar& = default;
  auto operator=(GuiCallBackRegistrar&&) -> GuiCallBackRegistrar& = delete;

  auto setRenderCallback(std::function<void(void)> newRenderFn) -> void override;
  auto render() -> void override;

  auto setReadyCallback(std::function<void(void)> newReadyFn) -> void override;
  auto ready() -> void override;

private:
  std::function<void(void)> renderFn;
  std::function<void(void)> readyCallback;
};

}
