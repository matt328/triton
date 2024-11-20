#include "EntityEditor.hpp"

#include "gp/components/Camera.hpp"
#include "ui/components/DialogManager.hpp"
#include "ui/components/ModalDialog.hpp"

#include "gp/components/EditorInfo.hpp"
#include "gp/components/Transform.hpp"
#include <entt/core/hashed_string.hpp>
#include <entt/entity/fwd.hpp>
#include <entt/entt.hpp>

namespace ed::ui::cmp {

   using entt::operator""_hs;

   constexpr auto DialogName = "AnimatedEntity";
   EntityEditor::EntityEditor(std::shared_ptr<tr::gp::IGameplaySystem> newGameplaySystem,
                              std::shared_ptr<data::DataFacade> newDataFacade,
                              std::shared_ptr<DialogManager> newDialogManager)
       : gameplaySystem{std::move(newGameplaySystem)},
         dataFacade{std::move(newDataFacade)},
         dialogManager{std::move(newDialogManager)} {
      Log.trace("Creating EntityEditor");
      createAnimatedEntityDialog();

      gameplaySystem->createTestEntity("Entity 1");
      gameplaySystem->createTestEntity("Entity 2");
      gameplaySystem->createTestEntity("Entity 3");
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

         auto& registry = gameplaySystem->getRegistry();
         auto view = registry.view<::tr::gp::cmp::EditorInfo>();
         for (auto [entity, editorInfo] : view.each()) {
            if (ImGui::Selectable(editorInfo.name.c_str(), entity == selectedEntity)) {
               Log.trace("Selected Entity: {}", editorInfo.name);
               selectedEntity = entity;
            }
         }

         ImGui::EndChild();
         ImGui::SameLine();

         // Right
         {
            // Header
            ImGui::BeginChild("item view", ImVec2(0, 0), ImGuiChildFlags_Border);
            if (selectedEntity.has_value()) {
               // Editor Info Component
               auto* editorInfo = registry.try_get<tr::gp::cmp::EditorInfo>(selectedEntity.value());
               if (editorInfo == nullptr) {
                  return;
               }
               ImGui::Text("%s", editorInfo->name.c_str());

               // Transform Component
               auto* transform = registry.try_get<tr::gp::cmp::Transform>(selectedEntity.value());
               if (transform != nullptr) {
                  if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
                     if (ImGui::DragFloat3("Position##Transform",
                                           glm::value_ptr(transform->position),
                                           0.5f)) {}
                     if (ImGui::DragFloat3("Rotation##Transform",
                                           glm::value_ptr(transform->rotation),
                                           0.5f)) {}
                  }
               }

               // Camera Component
               auto* camera = registry.try_get<tr::gp::cmp::Camera>(selectedEntity.value());
               if (camera != nullptr) {
                  if (ImGui::CollapsingHeader("Camera", ImGuiTreeNodeFlags_DefaultOpen)) {
                     if (ImGui::DragFloat3("Position##Camera",
                                           glm::value_ptr(camera->position),
                                           0.5f)) {}
                     if (ImGui::DragFloat3("Target##Camera",
                                           glm::value_ptr(camera->target),
                                           0.5f)) {}
                  }
               }

            } else {
               ImGui::Text("No Entity Selected");
            }
         }
         ImGui::EndChild();
      }
      ImGui::End();
   }

   void EntityEditor::createAnimatedEntityDialog() const {
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
