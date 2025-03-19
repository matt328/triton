#include "fx/GuiCallbackRegistrar.hpp"

namespace tr {

auto GuiCallBackRegistrar::setRenderCallback(std::function<void(void)> newRenderFn) -> void {
  renderFn = newRenderFn;
}

}
