#pragma once

#include "api/fx/IGuiCallbackRegistrar.hpp"
#include "fx/UIStateBuffer.hpp"

namespace tr {

class GuiCallBackRegistrar : public IGuiCallbackRegistrar {
public:
  GuiCallBackRegistrar(std::shared_ptr<UIStateBuffer> newStateBuffer);
  ~GuiCallBackRegistrar() override = default;

  GuiCallBackRegistrar(const GuiCallBackRegistrar&) = default;
  GuiCallBackRegistrar(GuiCallBackRegistrar&&) = delete;
  auto operator=(const GuiCallBackRegistrar&) -> GuiCallBackRegistrar& = default;
  auto operator=(GuiCallBackRegistrar&&) -> GuiCallBackRegistrar& = delete;

  auto setRenderCallback(std::function<void(void)> newRenderFn) -> void override;
  auto render(Timestamp t) -> void override;

  auto setReadyCallback(std::function<void(void)> newReadyFn) -> void override;
  auto ready() -> void override;

private:
  std::shared_ptr<UIStateBuffer> uiStateBuffer;

  std::function<void(void)> renderFn;
  std::function<void(void)> readyCallback;
};

}
