#include "FileControl.hpp"

#include "ui/components/FileDialog.hpp"

namespace ed {

FileControl::FileControl(std::string_view newName,
                         std::string_view newLabel,
                         std::shared_ptr<Properties> newProperties,
                         const std::vector<FilterItem>& filterItems)
    : name{newName.data()}, label{newLabel.data()} {
  fileDialog = std::make_unique<FileDialog>(std::move(newProperties), filterItems, name);
  fileDialog->setOnOk(
      [&](const std::vector<std::filesystem::path>& selections) { value = selections.front(); });
}

FileControl::~FileControl() {
}

auto FileControl::render(const DialogRenderContext& renderContext) -> void {
  float totalWidth = ImGui::CalcItemWidth();
  float buttonWidth = ImGui::GetFrameHeight();
  float textFieldWidth = totalWidth - buttonWidth - ImGui::GetStyle().ItemInnerSpacing.x;
  float buttonSize = ImGui::GetFrameHeight();

  std::string valueStr = value.string();

  ImGui::PushItemWidth(textFieldWidth);
  ImGui::InputText("##label", &valueStr);
  ImGui::PopItemWidth();

  ImGui::SameLine(0.f, 4.f); // Standard spacing

  if (ImGui::Button("...", ImVec2(buttonSize, buttonSize))) {
    const auto homeDir = sago::getDataHome();
    fileDialog->setOpen(std::nullopt);
  }

  ImGui::SameLine();
  ImGui::Text("%s", label.c_str());

  fileDialog->checkShouldOpen();
  fileDialog->render();
};

auto FileControl::getValue() const -> std::any {
  return value;
}

}
