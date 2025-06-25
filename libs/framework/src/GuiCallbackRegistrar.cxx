#include "fx/GuiCallbackRegistrar.hpp"

namespace tr {

auto GuiCallBackRegistrar::setRenderCallback(RenderFnType newRenderFn) -> void {
  renderFn = newRenderFn;
}

auto GuiCallBackRegistrar::render(std::optional<EditorState> editorState) -> void {
  assert(renderFn && "GuiCallback not registered");
  if (editorState) {
    renderFn(*editorState);
  }
}

auto GuiCallBackRegistrar::setReadyCallback(std::function<void(void)> newReadyFn) -> void {
  readyCallback = newReadyFn;
}

auto GuiCallBackRegistrar::ready() -> void {
  assert(readyCallback && "Gui Ready callback not registered");
  readyCallback();
}

}
