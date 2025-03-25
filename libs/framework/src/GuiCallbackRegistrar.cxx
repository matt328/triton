#include "fx/GuiCallbackRegistrar.hpp"

namespace tr {

auto GuiCallBackRegistrar::setRenderCallback(std::function<void(void)> newRenderFn) -> void {
  renderFn = newRenderFn;
}

auto GuiCallBackRegistrar::render() -> void {
  assert(renderFn && "GuiCallback not registered");
  renderFn();
}

}
