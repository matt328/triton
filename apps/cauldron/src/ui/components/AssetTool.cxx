#include "AssetTool.hpp"

namespace ed {

AssetTool::AssetTool() {
  Log.trace("Constructing Asset Tool");
}

AssetTool::~AssetTool() {
  Log.trace("Destroying AssetTool");
}

auto AssetTool::render() -> void {
  if (ImGui::Begin(ComponentName, nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {

    ImGui::Text("%s", "Asset Tool Here");
  }
  ImGui::End();
}

}
