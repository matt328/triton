#pragma once

#include "tr/GameplayFacade.hpp"
#include "data/DataFacade.hpp"
#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

// TODO tomorrow fire this back up
// Add a way to create a static model entity by selecting a model
// Figure out how to be able to position them using the editor, while saving their data in the
// editor's data store, and replicating it out to the engine.

// Add a way to create an animated model entity by selecting a model, skeleton, and animation
// Work on controllers, behaviors, scripts with the goal of controlling a model walking around the
// terrain

namespace ed::ui::components {
   struct EntityEditor {

      EntityEditor() = default;

      std::optional<std::string> selectedEntity{};

      void render([[maybe_unused]] tr::ctx::GameplayFacade& gameplayFacade,
                  [[maybe_unused]] data::DataFacade& dataFacade) {

         static auto position = glm::vec3{1.f};
         static auto previousPosition = position;
         static auto rotation = glm::identity<glm::quat>();
         static auto previousRotation = rotation;
         static auto showModelEntityModal = bool{};
         static auto showAnimatedModelModal = bool{};

         if (ImGui::Begin("Entity Editor",
                          nullptr,
                          ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_MenuBar)) {

            if (ImGui::BeginMenuBar()) {
               if (ImGui::BeginMenu("New")) {
                  if (ImGui::MenuItem("Static Model...")) {
                     showModelEntityModal = true;
                  }
                  if (ImGui::MenuItem("Animated Model...")) {
                     showAnimatedModelModal = true;
                  }
                  ImGui::EndMenu();
               }
               ImGui::EndMenuBar();
            }

            // Left
            ImGui::BeginChild("left pane",
                              ImVec2(150, 0),
                              ImGuiChildFlags_Border | ImGuiChildFlags_ResizeX);

            for (const auto& sceneData = dataFacade.getScene();
                 const auto& name : sceneData | std::views::keys) {
               if (ImGui::Selectable(name.c_str(), name == selectedEntity)) {
                  selectedEntity = name;
                  auto data = dataFacade.getEntityData(selectedEntity.value());
                  position = data.position;
                  previousPosition = position;
                  rotation = data.rotation;
                  previousRotation = data.rotation;
               }
            }

            ImGui::EndChild();
            ImGui::SameLine();

            // Right
            {
               ImGui::BeginChild("item view", ImVec2(0, 0), ImGuiChildFlags_Border);
               if (selectedEntity.has_value()) {
                  ImGui::Text("%s", selectedEntity.value().c_str());
               } else {
                  ImGui::Text("No Entity Selected");
               }

               if (selectedEntity.has_value()) {
                  ImGui::SeparatorText("Transform");
                  if (ImGui::DragFloat3("Position", glm::value_ptr(position), 1.f)) {
                     if (position != previousPosition) {
                        dataFacade.setEntityPosition(selectedEntity.value(), position);
                     }
                  }
                  ImGui::DragFloat3("Rotation", glm::value_ptr(rotation), .1f, -180.f, 180.f);
               }

               if (selectedEntity.has_value()) {
                  const auto entityData = dataFacade.getEntityData(selectedEntity.value());
                  if (!entityData.animations.empty()) {
                     renderAnimationArea(gameplayFacade);
                  }
               }
            }
            ImGui::EndChild();
         }
         ImGui::End();

         if (showModelEntityModal) {
            ImGui::OpenPopup("Static Model Entity");
            showModelEntityModal = false;
         }

         if (showAnimatedModelModal) {
            ImGui::OpenPopup("Animated Model Entity");
            showAnimatedModelModal = false;
         }

         renderStaticEntityDialog(dataFacade);
         renderAnimatedEntityDialog(dataFacade);
      }

      static void renderAnimatedEntityDialog(data::DataFacade& dataFacade) {
         if (ImGui::BeginPopupModal("Animated Model Entity")) {

            auto& models = dataFacade.getModels();
            auto modelNames = std::vector<const char*>{};
            modelNames.reserve(models.size());
            std::ranges::transform(models, std::back_inserter(modelNames), [](const auto& pair) {
               return pair.first.c_str();
            });

            auto& skeletons = dataFacade.getSkeletons();
            auto skeletonNames = std::vector<const char*>{};
            skeletonNames.reserve(skeletons.size());
            std::ranges::transform(skeletons,
                                   std::back_inserter(skeletonNames),
                                   [](const auto& pair) { return pair.first.c_str(); });

            auto& animations = dataFacade.getAnimations();
            auto animationNames = std::vector<const char*>{};
            animationNames.reserve(animations.size());
            std::ranges::transform(animations,
                                   std::back_inserter(animationNames),
                                   [](const auto& pair) { return pair.first.c_str(); });

            static int selectedModel = 0;
            static int selectedSkeleton = 0;
            static int selectedAnimation = 0;
            static auto entityName = std::string{"Unnamed Entity"};

            ImGui::InputText("Entity Name", &entityName);

            ImGui::Combo("Model",
                         &selectedModel,
                         modelNames.data(),
                         static_cast<int>(modelNames.size()));

            ImGui::Combo("Skeleton",
                         &selectedSkeleton,
                         skeletonNames.data(),
                         static_cast<int>(skeletonNames.size()));

            ImGui::Combo("Animation",
                         &selectedAnimation,
                         animationNames.data(),
                         static_cast<int>(animationNames.size()));

            if (ImGui::Button("Ok", ImVec2(120, 0))) {
               const auto& modelName = modelNames[selectedModel];
               const auto& skeletonName = skeletonNames[selectedSkeleton];
               const auto& animationName = animationNames[selectedAnimation];

               Log.debug("Beginning creating entity, modelName: {0}, skeletonName: {1}, "
                         "animationName: {2}",
                         modelName,
                         skeletonName,
                         animationName);
               try {
                  dataFacade.createAnimatedModel(entityName,
                                                 modelName,
                                                 skeletonName,
                                                 animationName);
               } catch (const std::exception& e) {
                  Log.error("Caught exception creating static model: {0}", e.what());
               }
               selectedModel = 0;
               ImGui::CloseCurrentPopup();
            }
            ImGui::SameLine();
            if (ImGui::Button("Cancel", ImVec2(120, 0))) {
               selectedModel = 0;
               ImGui::CloseCurrentPopup();
            }

            ImGui::EndPopup();
         }
      }

      static void renderStaticEntityDialog(data::DataFacade& dataFacade) {
         if (ImGui::BeginPopupModal("Static Model Entity")) {

            auto& models = dataFacade.getModels();
            auto modelNames = std::vector<const char*>{};
            modelNames.resize(models.size());

            std::ranges::transform(models, modelNames.begin(), [](const auto& pair) {
               return pair.first.c_str();
            });

            static int selectedModel = 0;
            static auto entityName = std::string{"Unnamed Entity"};

            ImGui::InputText("Entity Name", &entityName);

            ImGui::Combo("Model",
                         &selectedModel,
                         modelNames.data(),
                         static_cast<int>(modelNames.size()));

            if (ImGui::Button("Ok", ImVec2(120, 0))) {
               const auto& modelName = modelNames[selectedModel];
               Log.debug("Beginning creating entity");
               try {
                  dataFacade.createStaticModel(entityName, models.at(modelName).name);
               } catch (const std::exception& e) {
                  Log.error("Caught exception creating static model: {0}", e.what());
               }
               selectedModel = 0;
               ImGui::CloseCurrentPopup();
            }
            ImGui::SameLine();
            if (ImGui::Button("Cancel", ImVec2(120, 0))) {
               selectedModel = 0;
               ImGui::CloseCurrentPopup();
            }

            ImGui::EndPopup();
         }
      }

      static void renderAnimationArea([[maybe_unused]] tr::ctx::GameplayFacade& gameplayFacade) {
         ImGui::SeparatorText("Animation");
         static size_t itemCurrentIndex{};
         static bool playing{};
         static bool bindPose{};

         std::vector<const char*> items;
         items.push_back("Animation");

         if (ImGui::BeginCombo("Animation", items[itemCurrentIndex])) {
            for (size_t n = 0; n < items.size(); ++n) {
               const bool isSelected = (itemCurrentIndex == n);
               if (ImGui::Selectable(items[n], isSelected)) {
                  itemCurrentIndex = n;
                  Log.debug("Set Current Animation Name to {0}",
                            std::string{items[itemCurrentIndex]});
               }
               if (isSelected) {
                  ImGui::SetItemDefaultFocus();
               }
            }
            ImGui::EndCombo();
         }

         const bool previousBindPose = bindPose;

         ImGui::BeginDisabled(playing);
         if (ImGui::Checkbox("Bind Pose", &bindPose)) {
            if (bindPose != previousBindPose) {
               Log.debug("Set renderBindPose to {0}", bindPose);
            }
         }
         ImGui::EndDisabled();

         const bool previousPlaying = playing;

         ImGui::BeginDisabled(bindPose);

         ImGui::SameLine();

         if (ImGui::Checkbox("Play", &playing)) {
            if (playing != previousPlaying) {
               Log.debug("Set playing to {0}", playing);
            }
         }

         ImGui::BeginDisabled(playing);

         static float timeValue = 0.f;
         if (ImGui::SliderFloat("Time", &timeValue, 0.f, 1.f, "%.2f")) {
            Log.debug("Set timeValue to {0}", timeValue);
         }
         ImGui::EndDisabled();
         ImGui::EndDisabled();
      }
   };
}
