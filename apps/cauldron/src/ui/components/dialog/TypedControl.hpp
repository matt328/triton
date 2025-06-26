#pragma once

#include "ControlBase.hpp"
#include "ui/components/dialog/DialogContext.hpp"
#include <platform_folders.h>

namespace ed {

using ValueProvider = std::function<std::vector<std::string>(void)>;

template <typename T>
class TypedControl : public ControlBase {

public:
  TypedControl(std::string newName, std::string label, T initialValue)
      : name{std::move(newName)}, label(std::move(label)), value(initialValue) {
  }

  void render(const DialogRenderContext& renderContext = {}) override {
    static std::filesystem::path path{};
    if constexpr (std::is_same_v<T, std::string>) {
      if (renderContext.dropdownMap.contains(name)) {
        if (ImGui::BeginCombo(label.c_str(), value.c_str())) {
          for (const auto& option : renderContext.dropdownMap.at(name).items) {
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
    }
  }

  [[nodiscard]] auto getValue() const -> std::any override {
    return value;
  }

private:
  std::string name;
  std::string label;
  T value;
  DialogRenderContext renderContext;
};

enum class DialogResult : uint8_t {
  Ok = 1,
  Cancel
};

}
