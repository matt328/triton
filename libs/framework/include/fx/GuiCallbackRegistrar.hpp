#pragma once

#include "api/fx/IGuiCallbackRegistrar.hpp"
#include "bk/Chrono.h"

namespace tr {

class GuiCallBackRegistrar : public IGuiCallbackRegistrar {
public:
  GuiCallBackRegistrar() = default;
  ~GuiCallBackRegistrar() override = default;

  GuiCallBackRegistrar(const GuiCallBackRegistrar&) = default;
  GuiCallBackRegistrar(GuiCallBackRegistrar&&) = delete;
  auto operator=(const GuiCallBackRegistrar&) -> GuiCallBackRegistrar& = default;
  auto operator=(GuiCallBackRegistrar&&) -> GuiCallBackRegistrar& = delete;

  auto setRenderCallback(RenderFnType newRenderFn) -> void override;
  auto render(std::optional<EditorContextData> editorState) -> void override;

  auto setReadyCallback(std::function<void(void)> newReadyFn) -> void override;
  auto ready() -> void override;

private:
  RenderFnType renderFn;
  std::function<void(void)> readyCallback;
};

}
