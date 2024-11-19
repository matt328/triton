#include "EntityEditor.hpp"
#include "ui/components/DialogManager.hpp"
#include "ui/components/ModalDialog.hpp"
#include <string>

namespace ed::ui::cmp {
   constexpr auto DialogName = "AnimatedEntity";
   EntityEditor::EntityEditor(std::shared_ptr<tr::gp::IGameplaySystem> newGameplaySystem,
                              std::shared_ptr<data::DataFacade> newDataFacade,
                              std::shared_ptr<DialogManager> newDialogManager)
       : gameplaySystem{std::move(newGameplaySystem)},
         dataFacade{std::move(newDataFacade)},
         dialogManager{std::move(newDialogManager)} {
      Log.trace("Creating EntityEditor");
      createAnimatedEntityDialog();
   }

   EntityEditor::~EntityEditor() {
      Log.trace("Destroying EntityEditor");
   }

   void EntityEditor::render() {

      if (ImGui::Begin("Entity Editor",
                       nullptr,
                       ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_MenuBar)) {

         if (ImGui::BeginMenuBar()) {
            if (ImGui::BeginMenu("New")) {
               if (ImGui::MenuItem("Static Model...")) {
                  // dialogManager->showDialog(StaticModelDialogName)
               }
               if (ImGui::MenuItem("Animated Model...")) {
                  dialogManager->setOpen(DialogName);
               }
               ImGui::EndMenu();
            }
            ImGui::EndMenuBar();
         }

         // Left
         ImGui::BeginChild("left pane",
                           ImVec2(150, 0),
                           ImGuiChildFlags_Border | ImGuiChildFlags_ResizeX);

         // for (const auto& sceneData = dataFacade.getScene();
         //      const auto& name : sceneData | std::views::keys) {
         //    if (ImGui::Selectable(name.c_str(), name == selectedEntity)) {
         //       selectedEntity = name;
         //       auto data = dataFacade.getEntityData(selectedEntity.value());
         //       position = data.position;
         //       previousPosition = position;
         //       rotation = data.rotation;
         //       previousRotation = data.rotation;
         //    }
         // }

         ImGui::EndChild();
         ImGui::SameLine();

         // Right
         // {
         //    ImGui::BeginChild("item view", ImVec2(0, 0), ImGuiChildFlags_Border);
         //    if (selectedEntity.has_value()) {
         //       ImGui::Text("%s", selectedEntity.value().c_str());
         //    } else {
         //       ImGui::Text("No Entity Selected");
         //    }

         //    if (selectedEntity.has_value()) {
         //       ImGui::SeparatorText("Transform");
         //       if (ImGui::DragFloat3("Position", glm::value_ptr(position), 1.f)) {
         //          if (position != previousPosition) {
         //             dataFacade.setEntityPosition(selectedEntity.value(), position);
         //          }
         //       }
         //       ImGui::DragFloat3("Rotation", glm::value_ptr(rotation), .1f, -180.f, 180.f);
         //    }

         //    if (selectedEntity.has_value()) {
         //       const auto entityData = dataFacade.getEntityData(selectedEntity.value());
         //       if (!entityData.animations.empty()) {
         //          renderAnimationArea(gameplayFacade);
         //       }
         //    }
         // }
         // ImGui::EndChild();
      }
      ImGui::End();
   }

   void EntityEditor::createAnimatedEntityDialog() {
      // Maybe have to register a data provider thing to pass in a lambda to evaluate when
      // actually rendering this dialog. As it is right now, these will always be empty.
      ValueProvider modelProvider = [this]() -> std::vector<std::string> {
         const auto& models = dataFacade->getModels();
         auto modelNames = std::vector<std::string>{};
         modelNames.reserve(models.size());
         std::ranges::transform(models, std::back_inserter(modelNames), [](const auto& pair) {
            return pair.first;
         });
         return modelNames;
      };

      ValueProvider skeletonProvider = [this]() -> std::vector<std::string> {
         const auto& skeletons = dataFacade->getSkeletons();
         auto skeletonNames = std::vector<std::string>{};
         skeletonNames.reserve(skeletons.size());
         std::ranges::transform(skeletons, std::back_inserter(skeletonNames), [](const auto& pair) {
            return pair.first;
         });
         return skeletonNames;
      };

      ValueProvider animationsProvider = [this]() -> std::vector<std::string> {
         const auto& animations = dataFacade->getAnimations();
         auto animationNames = std::vector<std::string>{};
         animationNames.reserve(animations.size());
         std::ranges::transform(animations,
                                std::back_inserter(animationNames),
                                [](const auto& pair) { return pair.first; });
         return animationNames;
      };

      const auto onOk = [&](const cmp::ModalDialog& dialog) {
         dataFacade->createAnimatedModel(
             data::EntityData{.name = dialog.getValue<std::string>("name").value(),
                              .modelName = dialog.getValue<std::string>("model").value(),
                              .skeleton = dialog.getValue<std::string>("skeleton").value(),
                              .animations = {dialog.getValue<std::string>("animation").value()}});
      };

      const auto onCancel = []() { Log.debug("Cancelled Dialog with no input"); };

      auto dialog = std::make_unique<ModalDialog>(DialogName, onOk, onCancel);

      dialog->addControl("name", "Entity Name", std::string{"Unnamed Entity"});

      dialog->addControl("model", "Model Name", std::string{"Unnamed Model"}, modelProvider);

      dialog->addControl("skeleton",
                         "Skeleton Name",
                         std::string{"Unnamed Skeleton"},
                         skeletonProvider);

      dialog->addControl("animation",
                         "Animation Name",
                         std::string{"Unnamed Animation"},
                         animationsProvider);

      dialogManager->addDialog(DialogName, std::move(dialog));
   }
}
