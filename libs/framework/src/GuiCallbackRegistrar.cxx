#include "fx/GuiCallbackRegistrar.hpp"

namespace tr {

GuiCallBackRegistrar::GuiCallBackRegistrar(std::shared_ptr<UIStateBuffer> newStateBuffer)
    : uiStateBuffer{std::move(newStateBuffer)} {
}

auto GuiCallBackRegistrar::setRenderCallback(std::function<void(void)> newRenderFn) -> void {
  renderFn = newRenderFn;
}

auto GuiCallBackRegistrar::render(Timestamp t) -> void {
  auto uiState = uiStateBuffer->getStates(t);
  assert(renderFn && "GuiCallback not registered");
  renderFn();
}

auto GuiCallBackRegistrar::setReadyCallback(std::function<void(void)> newReadyFn) -> void {
  readyCallback = newReadyFn;
}

auto GuiCallBackRegistrar::ready() -> void {
  assert(readyCallback && "Gui Ready callback not registered");
  readyCallback();
}

}
