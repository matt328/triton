#pragma once

#include "tr/GameplayFacade.hpp"
#include "data/DataFacade.hpp"

// TODO tomorrow fire this back up
// Add a way to create a static model entity by selecting a model
// Figure out how to be able to position them using the editor, while saving their data in the
// editor's data store, and replicating it out to the engine.

// Add a way to create an animated model entity by selecting a model, skeleton, and animation
// Work on controllers, behaviors, scripts with the goal of controlling a model walking around the
// terrain

namespace ed::ui::components {
   struct EntityEditor {

      EntityEditor() {
      }

      std::optional<std::string> selectedEntity{};

      void render([[maybe_unused]] tr::ctx::GameplayFacade& gameplayFacade,
                  [[maybe_unused]] data::DataFacade& dataFacade) {

         const auto unsaved = dataFacade.isUnsaved() ? ImGuiWindowFlags_UnsavedDocument : 0;

         static auto position = glm::vec3{1.f};
         static auto previousPosition = position;
         static auto rotation = glm::identity<glm::quat>();
         static auto previousRotation = rotation;

         if (ImGui::Begin("Entity Editor", nullptr, ImGuiWindowFlags_MenuBar | unsaved)) {

            if (ImGui::BeginMenuBar()) {
               if (ImGui::BeginMenu("New")) {
                  if (ImGui::MenuItem("Entity...")) {
                     dataFacade.createEntity("Test Entity");
                  }
                  if (ImGui::MenuItem("Entity #2...")) {
                     dataFacade.createEntity("Test Entity #2");
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
                           Log::debug << "Adding component: " << name << std::endl;
                           // TODO: Create Modals to Create different components.
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
