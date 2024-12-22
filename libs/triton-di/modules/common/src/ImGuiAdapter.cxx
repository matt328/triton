#include "cm/ImGuiAdapter.hpp"

namespace tr {

   auto ImGuiAdapter::needsKeyboard() -> bool {
      return ImGui::GetIO().WantCaptureKeyboard || ImGui::GetIO().WantTextInput;
   }

   auto ImGuiAdapter::needsMouse() -> bool {
      return ImGui::GetIO().WantCaptureMouse;
   }

   auto ImGuiAdapter::disableMouse() -> void {
      ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NoMouse;
   }

   auto ImGuiAdapter::enableMouse() -> void {
      ImGui::GetIO().ConfigFlags &= ~ImGuiConfigFlags_NoMouse;
   }

}