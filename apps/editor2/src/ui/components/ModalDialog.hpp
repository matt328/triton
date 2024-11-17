#pragma once

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
      static constexpr auto SkeletonFilters = std::array{nfdfilteritem_t{"Ozz Skeleton", "ozz"}};

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
               auto inPath = NFD::UniquePath{};
               if (const auto result =
                       OpenDialog(inPath, SkeletonFilters.data(), SkeletonFilters.size());
                   result == NFD_OKAY) {
                  value = std::filesystem::path{std::string{inPath.get()}};
               } else if (result == NFD_CANCEL) {
                  Log.info("User pressed cancel");
               } else {
                  Log.error("Error selecting skeleton file: {0}", NFD::GetError());
               }
            }
            ImGui::SameLine();
            ImGui::Text("%s", label.c_str());
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

   class ModalDialog;

   using OkFunction = std::function<void(const ModalDialog&)>;
   using CancelFunction = std::function<void(void)>;

   class ModalDialog {
    public:
      explicit ModalDialog(std::string title,
                           const OkFunction& okFunction,
                           const CancelFunction& cancelFunction)
          : title(std::move(title)),
            onOk{std::make_optional(okFunction)},
            onCancel{std::make_optional(cancelFunction)} {
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
      std::string title{};
      std::map<std::string, std::unique_ptr<ControlBase>> controls;
      std::optional<OkFunction> onOk;
      std::optional<CancelFunction> onCancel;
   };

};
