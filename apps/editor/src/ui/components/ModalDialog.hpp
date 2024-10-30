#pragma once

#include <any>
#include <utility>

namespace ed::ui::cmp {

   class ControlBase {
    public:
      ControlBase() = default;
      ControlBase(const ControlBase&) = default;
      ControlBase(ControlBase&&) = delete;
      auto operator=(const ControlBase&) -> ControlBase& = default;
      auto operator=(ControlBase&&) -> ControlBase& = delete;

      virtual ~ControlBase() = default;
      virtual void render() = 0;
      [[nodiscard]] virtual auto getValue() const -> std::any = 0;
   };

   template <typename T>
   class TypedControl : public ControlBase {
    public:
      TypedControl(std::string label, T initialValue)
          : label(std::move(label)), value(initialValue) {
      }

      void render() override {
         if constexpr (std::is_same_v<T, std::string>) {
            ImGui::InputText(label.c_str(), &value);
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
      std::string label;
      T value;
   };

   enum class DialogResult : uint8_t {
      Ok = 1,
      Cancel
   };

   class ModalDialog {
    public:
      explicit ModalDialog(std::string title) : title(std::move(title)) {
      }

      template <typename T>
      void addControl(const std::string& name, const std::string& label, T initialValue) {
         controls[name] = std::make_unique<TypedControl<T>>(label, initialValue);
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

      [[nodiscard]] auto render() -> DialogResult {

         if (!isOpen) {
            return DialogResult::Cancel;
         }

         bool shouldOk{};
         bool shouldCancel{};

         if (ImGui::BeginPopupModal(title.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
            for (auto& [name, control] : controls) {
               control->render();
            }
            if (ImGui::Button("OK")) {
               shouldOk = true;
            }
            ImGui::SameLine();
            if (ImGui::Button("Cancel")) {
               shouldCancel = true;
            }

            ImGui::EndPopup();
         }

         if (shouldOk) {
            ImGui::CloseCurrentPopup();
            isOpen = false;
            return DialogResult::Ok;
         }

         if (shouldCancel) {
            ImGui::CloseCurrentPopup();
            isOpen = false;
            return DialogResult::Cancel;
         }
         return DialogResult::Cancel;
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
      std::string title{};
      std::map<std::string, std::unique_ptr<ControlBase>> controls;
   };

};
