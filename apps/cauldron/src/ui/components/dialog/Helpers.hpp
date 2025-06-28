#pragma once

#include "ui/assets/IconsLucide.hpp"
#include "ui/components/FileDialog.hpp"

namespace ed {

inline static auto renderOkCancelButtons(float buttonWidth = 80.f) -> std::pair<bool, bool> {
  bool ok = false;
  bool cancel = false;

  auto availableWidth = ImGui::GetContentRegionAvail().x;
  ImGui::SetCursorPosX(availableWidth - (buttonWidth * 2));

  if (ImGui::Button(ICON_LC_CIRCLE_CHECK_BIG " OK", ImVec2(buttonWidth, 0.f))) {
    ok = true;
  }
  ImGui::SameLine();
  if (ImGui::Button(ICON_LC_BAN " Cancel", ImVec2(buttonWidth, 0.f))) {
    cancel = true;
  }
  return {ok, cancel};
}

inline static auto renderFileControl(const std::string& name,
                                     std::shared_ptr<Properties> properties,
                                     const std::vector<FilterItem>& filterItems,
                                     std::filesystem::path& value) -> void {
  using DialogMap = std::unordered_map<std::string, std::unique_ptr<FileDialog>>;
  static DialogMap dialogs;
  static std::unordered_map<std::string, std::optional<std::filesystem::path>> selectedPaths;

  // Create dialog for this control if it doesn't exist yet
  auto& dialog =
      dialogs.try_emplace(name, std::make_unique<FileDialog>(properties, filterItems, name))
          .first->second;

  // Check for selection
  if (auto result = dialog->getFinalSelection(); result.has_value()) {
    selectedPaths[name] = result;
    value = *result;
  }

  // Render UI
  float totalWidth = ImGui::CalcItemWidth();
  float buttonWidth = ImGui::GetFrameHeight();
  float textFieldWidth = totalWidth - buttonWidth - ImGui::GetStyle().ItemInnerSpacing.x;
  float buttonSize = ImGui::GetFrameHeight();

  auto valueStr = value.string();

  ImGui::PushItemWidth(textFieldWidth);
  ImGui::InputText("##label", &valueStr);
  ImGui::PopItemWidth();

  ImGui::SameLine(0.f, 4.f);

  if (ImGui::Button("...", ImVec2(buttonSize, buttonSize))) {
    dialog->setOpen(std::nullopt);
  }

  ImGui::SameLine();
  ImGui::Text("File");

  dialog->checkShouldOpen();
  dialog->render();
}

}
