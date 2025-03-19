#pragma once

#include "fx/IGuiCallbackRegistrar.hpp"

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

private:
  std::function<void(void)> renderFn;
};

}
