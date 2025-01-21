#pragma once

#include "ControlBase.hpp"
#include "ui/components/FileDialog.hpp"

namespace ed {

using ValueProvider = std::function<std::vector<std::string>(void)>;

template <typename T>
class TypedControl : public ControlBase {
  static constexpr auto SkeletonFilters = std::array{nfdfilteritem_t{"Ozz Skeleton", "ozz"}};

public:
  TypedControl(std::string label,
               T initialValue,
               std::optional<ValueProvider> newValueProvider = std::nullopt)
      : label(std::move(label)), value(initialValue), valueProvider{std::move(newValueProvider)} {
    fileDialog = std::make_unique<FileDialog>();
  }

  void render() override {
    static std::filesystem::path path{};
    if constexpr (std::is_same_v<T, std::string>) {
      if (valueProvider.has_value()) {
        // Render combobox if options are provided
        if (ImGui::BeginCombo(label.c_str(), value.c_str())) {
          for (const auto& option : valueProvider.value()()) {
            bool isSelected = (value == option);
            if (ImGui::Selectable(option.c_str(), isSelected)) {
              value = option;
            }
            if (isSelected) {
              ImGui::SetItemDefaultFocus();
            }
          }
          ImGui::EndCombo();
        }
      } else {
        ImGui::InputText(label.c_str(), &value);
      }
    } else if constexpr (std::is_same_v<T, int>) {
      ImGui::InputInt(label.c_str(), &value);
    } else if constexpr (std::is_same_v<T, float>) {
      ImGui::InputFloat(label.c_str(), &value);
    } else if constexpr (std::is_same_v<T, glm::vec3>) {
      ImGui::SliderFloat3(label.c_str(), (float*)&value, -5.f, 5.f);
    } else if constexpr (std::is_same_v<T, std::filesystem::path>) {

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
        fileDialog->setOpen(std::filesystem::path{R"(C:\Users\Matt\Projects\game-assets)"});
      }

      ImGui::SameLine();
      ImGui::Text("%s", label.c_str());
    }

    fileDialog->checkShouldOpen();

    fileDialog->render();
  }

  [[nodiscard]] auto getValue() const -> std::any override {
    return value;
  }

private:
  std::string label;
  T value;
  std::optional<ValueProvider> valueProvider;

  std::unique_ptr<FileDialog> fileDialog;
};

enum class DialogResult : uint8_t {
  Ok = 1,
  Cancel
};

}
