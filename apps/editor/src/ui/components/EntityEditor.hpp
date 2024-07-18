#pragma once

#include "cm/EntitySystemTypes.hpp"

#include "tr/GameplayFacade.hpp"
#include "data/DataFacade.hpp"

#include <imgui.h>
#include <imgui_stdlib.h>

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

         const auto unsaved = dataFacade.isUnsaved() ? ImGuiWindowFlags_UnsavedDocument : 0;

         static auto position = glm::vec3{1.f};
         static auto previousPosition = position;
         static auto rotation = glm::identity<glm::quat>();
         static auto previousRotation = rotation;
         static auto showModelEntityModal = bool{};

         if (ImGui::Begin("Entity Editor", nullptr, ImGuiWindowFlags_MenuBar | unsaved)) {

            if (ImGui::BeginMenuBar()) {
               if (ImGui::BeginMenu("New")) {
                  if (ImGui::MenuItem("Static Model...")) {
                     showModelEntityModal = true;
                  }
                  if (ImGui::MenuItem("Entity #2...")) {
                     // dataFacade.createEntity("Test Entity #2");
                  }
                  ImGui::EndMenu();
               }
               ImGui::EndMenuBar();
            }

            // Left
            ImGui::BeginChild("left pane",
                              ImVec2(150, 0),
                              ImGuiChildFlags_Border | ImGuiChildFlags_ResizeX);

            const auto& sceneData = dataFacade.getScene();

            for (const auto& [name, entityData] : sceneData) {
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
               ImGui::BeginGroup();
               ImGui::BeginChild("item view",
                                 ImVec2(0,
                                        -ImGui::GetFrameHeightWithSpacing() *
                                            4)); // Leave room for 1 line below us

               if (selectedEntity.has_value()) {
                  ImGui::Text("%s", selectedEntity.value().c_str());

                  if (ImGui::Button("Add..")) {
                     ImGui::OpenPopup("component_popup");
                  }

                  const std::array<std::string, 4> names = {"Model",
                                                            "Animated Model",
                                                            "Script",
                                                            "Behavior"};

                  if (ImGui::BeginPopup("component_popup")) {
                     ImGui::SeparatorText("Components");
                     for (const auto& name : names) {
                        if (ImGui::Selectable(name.c_str())) {
                           Log.debug("Adding Component: {0}", name);
                           if (name == "Model") {
                              showModelEntityModal = true;
                           }
                           // TODO: Create Modals to Create different components.
                           /* Once a Model component is added to an entity, should trigger the model
                              to be loaded by the engine.
                              Need to think about how to map entities in the editor's data store to
                              cm::Entity.  This information will be temporary, and won't be saved
                              with the project since the entity ids aren't predictable.
                              The entityId to entity name map should be created on demand, and
                              updated as the dataFacade sends things to the gameplayFacade.
                              Maybe dataFacade should own the mapping.
                           */
                        }
                     }
                     ImGui::EndPopup();
                  }

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
            }
            ImGui::EndChild();
            ImGui::EndGroup();
         }
         ImGui::End();

         if (showModelEntityModal) {
            ImGui::OpenPopup("Static Model Entity");
            showModelEntityModal = false;
         }

         if (ImGui::BeginPopupModal("Static Model Entity")) {

            auto& models = dataFacade.getModels();
            auto modelNames = std::vector<const char*>{};
            modelNames.resize(models.size());

            std::transform(models.begin(), models.end(), modelNames.begin(), [](const auto& pair) {
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
               Log.debug("Selected Model: {0}", modelName);

               /*
                  Have the data facade own the future monitor, it can just set it's own state
                  of 'busy' and the manager and ui can respond to that to show a progress spinner.
                  Since the data facadw owns the future monitor, it can attach lambdas to run when
                  futures finish to populate the map<entityName to entityId> and outside the facade
                  should never know about entity ids as the facade will take in names and translate
                  them

               */
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

      void renderAnimationArea([[maybe_unused]] tr::ctx::GameplayFacade& gameplayFacade) {
         ImGui::SeparatorText("Animation");
         // static size_t itemCurrentIndex{};
         // static bool playing{};
         // static bool bindPose{};

         // std::vector<const char*> items;
         // items.push_back("Animation");

         //    if (ImGui::BeginCombo("Animation Name", items[itemCurrentIndex])) {
         //       for (size_t n = 0; n < items.size(); ++n) {
         //          const bool isSelected = (itemCurrentIndex == n);
         //          if (ImGui::Selectable(items[n], isSelected)) {
         //             itemCurrentIndex = n;
         //             animationComponent.currentAnimationName =
         //             std::string{items[itemCurrentIndex]};
         //          }
         //          if (isSelected) {
         //             ImGui::SetItemDefaultFocus();
         //          }
         //       }
         //       ImGui::EndCombo();
         //    }

         //    bool previousBindPose = bindPose;

         //    ImGui::BeginDisabled(playing);
         //    if (ImGui::Checkbox("Bind Pose", &bindPose)) {
         //       if (bindPose != previousBindPose) {
         //          animationComponent.renderBindPose = bindPose;
         //       }
         //    }
         //    ImGui::EndDisabled();

         //    bool previousPlaying = playing;

         //    ImGui::BeginDisabled(bindPose);

         //    if (ImGui::Checkbox("Play", &playing)) {
         //       if (playing != previousPlaying) {
         //          animationComponent.playing = playing;
         //       }
         //    }

         //    ImGui::BeginDisabled(playing);
         //    const auto [min_v, max_v] =
         //        gameplayFacade.getAnimationTimeRange(animationComponent.animationHandle);

         //    static float timeValue = min_v;
         //    if (ImGui::SliderFloat("Time", &timeValue, min_v, max_v, "%.2f")) {
         //       const auto value = (timeValue - min_v) / (max_v - min_v);
         //       animationComponent.timeRatio = value;
         //    }
         //    ImGui::EndDisabled();
         //    ImGui::EndDisabled();
         // }
      };
   };
}
