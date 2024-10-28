#include <utility>

#pragma once

namespace ed::ui::cmp {

   class ModalDialog {
    public:
      // Supported input types
      using InputType = std::variant<int, float, std::string>;

      // Constructor with the name of the modal
      explicit ModalDialog(std::string title) : title(std::move(title)) {
      }

      // Add an input field to the dialog
      void addInput(const std::string& name, InputType defaultValue) {
         inputs[name] = std::move(defaultValue);
      }

      // Show the modal dialog
      auto show() -> std::optional<std::unordered_map<std::string, InputType>> {
         if (isOpen) {
            ImGui::OpenPopup(title.c_str());
            isOpen = false;
         }

         std::optional<std::unordered_map<std::string, InputType>> result;

         if (ImGui::BeginPopupModal(title.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
            // Render inputs
            for (auto& [name, value] : inputs) {
               std::visit(
                   [&name](auto&& val) {
                      using T = std::decay_t<decltype(val)>;
                      if constexpr (std::is_same_v<T, int>) {
                         ImGui::InputInt(name.c_str(), &val);
                      } else if constexpr (std::is_same_v<T, float>) {
                         ImGui::InputFloat(name.c_str(), &val);
                      } else if constexpr (std::is_same_v<T, std::string>) {
                         char buffer[256];
                         std::strncpy(buffer, val.c_str(), sizeof(buffer));
                         if (ImGui::InputText(name.c_str(), buffer, sizeof(buffer))) {
                            val = buffer;
                         }
                      }
                   },
                   value);
            }

            // OK and Cancel buttons
            if (ImGui::Button("OK")) {
               isOkPressed = true;
               ImGui::CloseCurrentPopup();
            }
            ImGui::SameLine();
            if (ImGui::Button("Cancel")) {
               isCancelPressed = true;
               ImGui::CloseCurrentPopup();
            }

            ImGui::EndPopup();
         }

         // Return results if OK or Cancel was pressed
         if (isOkPressed) {
            result = inputs;
            resetDialogState();
         } else if (isCancelPressed) {
            result = std::nullopt;
            resetDialogState();
         }

         return result;
      }

      // Open the dialog
      void open() {
         isOpen = true;
      }

    private:
      std::string title;
      std::unordered_map<std::string, InputType> inputs;
      bool isOpen{};
      bool isOkPressed{};
      bool isCancelPressed{};

      void resetDialogState() {
         isOkPressed = false;
         isCancelPressed = false;
      }
   };

};
