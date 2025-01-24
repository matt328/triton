#pragma once

#include "Properties.hpp"
#include "TypedControl.hpp"
#include "imgui.h"
#include "ui/components/FileDialog.hpp"
#include "ui/components/dialog/FileControl.hpp"

namespace ed {

class ModalDialog;

using OkFunction = std::function<void(const ModalDialog&)>;
using CancelFunction = std::function<void(void)>;

class ModalDialog {
public:
  explicit ModalDialog(const char* icon,
                       std::string title,
                       const OkFunction& okFunction,
                       const CancelFunction& cancelFunction)
      : title(std::string{icon} + title),
        onOk{std::make_optional(okFunction)},
        onCancel{std::make_optional(cancelFunction)} {
  }

  template <typename T>
  void addControl(const std::string& name,
                  const std::string& label,
                  T initialValue,
                  std::optional<ValueProvider> valueProvider = std::nullopt,
                  std::optional<std::shared_ptr<Properties>> properties = std::nullopt) {
    controls[name] =
        std::make_unique<TypedControl<T>>(label, initialValue, valueProvider, properties);
  }

  void addFileControl(const std::string& name,
                      const std::string& label,
                      std::shared_ptr<Properties> newProperties,
                      const std::vector<FilterItem>& filterItems) {
    controls[name] = std::make_unique<FileControl>(label, newProperties, filterItems);
  }

  template <typename T>
  auto getValue(const std::string& name) const -> std::optional<T> {
    if (controls.contains(name)) {
      auto it = controls.find(name);
      try {
        return std::any_cast<T>(it->second->getValue());
      } catch (const std::bad_any_cast& bac) {
        Log.warn("Not what you were expecting, hmm? Fat Lemongrab!");
      }
    }
    return std::nullopt;
  }

  void render() {
    if (!isOpen) {
      return;
    }

    bool shouldOk{};
    bool shouldCancel{};

    if (ImGui::BeginPopupModal(title.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
      for (auto& [name, control] : controls) {
        control->render();
      }

      ImGui::Separator();

      auto availableWidth = ImGui::GetContentRegionAvail().x;
      auto buttonWidth = 80.f;

      ImGui::SetCursorPosX(availableWidth - buttonWidth * 2);

      if (ImGui::Button(ICON_LC_CIRCLE_CHECK_BIG " OK", ImVec2(buttonWidth, 0.f))) {
        shouldOk = true;
      }
      ImGui::SameLine();
      if (ImGui::Button(ICON_LC_BAN " Cancel", ImVec2(buttonWidth, 0.f))) {
        shouldCancel = true;
      }

      ImGui::EndPopup();
    }

    if (shouldOk) {
      ImGui::CloseCurrentPopup();
      isOpen = false;
      if (onOk.has_value()) {
        onOk.value()(*this);
      }
    }

    if (shouldCancel) {
      ImGui::CloseCurrentPopup();
      isOpen = false;
      if (onCancel.has_value()) {
        onCancel.value()();
      }
    }
  }

  void setOpen() {
    isOpen = true;
  }

  void checkShouldOpen() {
    if (isOpen) {
      ImGui::OpenPopup(title.c_str());
    }
  }

private:
  bool isOpen{};
  std::string title;
  std::map<std::string, std::unique_ptr<ControlBase>> controls;
  std::optional<OkFunction> onOk;
  std::optional<CancelFunction> onCancel;
};

};
