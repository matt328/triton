#include "fx/GuiCallbackRegistrar.hpp"

namespace tr {

auto GuiCallBackRegistrar::setRenderCallback(std::function<void(void)> newRenderFn) -> void {
  renderFn = newRenderFn;
}

auto GuiCallBackRegistrar::render() -> void {
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
