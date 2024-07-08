#pragma once

#include "tr/GameplayFacade.hpp"
#include "data/DataFacade.hpp"

namespace ed::ui::components {
   struct EntityEditor {

      EntityEditor(const std::function<void(uint32_t)>& loadTerrain) : onLoadTerrain{loadTerrain} {
      }

      std::optional<uint32_t> selectedEntity{};
      std::function<void(uint32_t size)> onLoadTerrain;

      void render([[maybe_unused]] tr::ctx::GameplayFacade& gameplayFacade,
                  [[maybe_unused]] data::DataFacade& dataFacade) {
         // Entity Editor
         /*
         This editor directly altered the properties inside the ECS. This won't work like this
         anymore, as the editor has its own data store, and the data flow is only one way from
         editor store into ECS. Nothing that changes inside the ECS should propagate back out to the
         editor's data store.

         auto& es = gameplayFacade.getAllEntities();

         const auto unsaved = dataFacade.isUnsaved() ? ImGuiWindowFlags_UnsavedDocument : 0;

         if (ImGui::Begin("Entity Editor", nullptr, ImGuiWindowFlags_MenuBar | unsaved)) {

            if (ImGui::BeginMenuBar()) {
               if (ImGui::BeginMenu("New")) {
                  if (ImGui::MenuItem("Entity...")) {}
                  ImGui::EndMenu();
               }
               ImGui::EndMenuBar();
            }

            // Left
            ImGui::BeginChild("left pane",
                              ImVec2(150, 0),
                              ImGuiChildFlags_Border | ImGuiChildFlags_ResizeX);
            for (auto e : es) {
               auto& infoComponent = gameplayFacade.getEditorInfo(e);
               if (ImGui::Selectable(infoComponent.name.c_str(),
                                     static_cast<uint32_t>(e) == selectedEntity)) {
                  selectedEntity = static_cast<uint32_t>(e);
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
                  ImGui::Text("Entity ID: %d", selectedEntity.value());
               } else {
                  ImGui::Text("No Entity Selected");
               }

               if (selectedEntity.has_value()) {
                  const auto maybeTransform = gameplayFacade.getComponent<tr::gp::ecs::Transform>(
                      static_cast<entt::entity>(selectedEntity.value()));

                  const auto maybeDebug = gameplayFacade.getComponent<tr::gp::ecs::DebugConstants>(
                      static_cast<entt::entity>(selectedEntity.value()));

                  const auto maybeAnimated = gameplayFacade.getComponent<tr::gp::ecs::Animation>(
                      static_cast<entt::entity>(selectedEntity.value()));

                  if (maybeTransform.has_value()) {
                     auto& transform = maybeTransform.value().get();
                     ImGui::SeparatorText("Transform");
                     ImGui::DragFloat3("Position", glm::value_ptr(transform.position), 1.f);
                     ImGui::DragFloat3("Rotation",
                                       glm::value_ptr(transform.rotation),
                                       .1f,
                                       -180.f,
                                       180.f);
                  }
                  if (maybeDebug.has_value()) {
                     auto& d = maybeDebug.value().get();
                     ImGui::SeparatorText("DebugConstants");
                     ImGui::DragFloat("Specular Power", &d.specularPower, 0.5f);
                  }

                  if (maybeAnimated.has_value()) {
                     auto& animationComponent = maybeAnimated.value().get();
                     renderAnimationArea(gameplayFacade, animationComponent);
                  }
               }
               ImGui::EndChild();

               if (ImGui::Button("Load Terrain")) {
                  onLoadTerrain(512);
               }
               ImGui::SameLine();
               if (ImGui::Button("Load Cube")) {
                  const auto modelName = std::filesystem::path{
                      R"(C:\Users\Matt\Projects\game-assets\models\cube\cube2.gltf)"};
                  const auto skeletonPath = std::filesystem::path{
                      R"(C:\Users\Matt\Projects\game-assets\models\cube\skeleton.ozz)"};
                  const auto animationPath = std::filesystem::path{
                      R"(C:\Users\Matt\Projects\game-assets\models\cube\animation.ozz)"};
                  // skinnedModelFutures.push_back(
                  //     gameplayFacade.loadSkinnedModelAsync(modelName, skeletonPath,
                  //     animationPath));
               }
               if (ImGui::Button("Load Cesium")) {
                  const auto modelName = std::filesystem::path{
                      R"(C:\Users\Matt\Projects\game-assets\models\cesiumman\CesiumMan.gltf)"};
                  const auto skeletonPath = std::filesystem::path{
                      R"(C:\Users\Matt\Projects\game-assets\models\cesiumman\skeleton.ozz)"};
                  const auto animationPath = std::filesystem::path{
                      R"(C:\Users\Matt\Projects\game-assets\models\cesiumman\animation.ozz)"};
                  // skinnedModelFutures.push_back(
                  //     gameplayFacade.loadSkinnedModelAsync(modelName, skeletonPath,
                  //     animationPath));
               }

               if (ImGui::Button("Load Cesium Reexport")) {
                  const auto modelName = std::filesystem::path{
                      R"(C:\Users\Matt\Projects\game-assets\models\cesiumman\reexport\CesiumManReexported.gltf)"};
                  const auto skeletonPath = std::filesystem::path{
                      R"(C:\Users\Matt\Projects\game-assets\models\cesiumman\reexport\skeleton.ozz)"};
                  const auto animationPath = std::filesystem::path{
                      R"(C:\Users\Matt\Projects\game-assets\models\cesiumman\reexport\animation.ozz)"};
                  // skinnedModelFutures.push_back(
                  //     gameplayFacade.loadSkinnedModelAsync(modelName, skeletonPath,
                  //     animationPath));
               }

               if (ImGui::Button("Load Peasant")) {
                  const auto modelName = std::filesystem::path{
                      R"(C:\Users\Matt\Projects\game-assets\models\gltf-working\walking.gltf)"};
                  const auto skeletonPath = std::filesystem::path{
                      R"(C:\Users\Matt\Projects\game-assets\models\gltf-working\skeleton.ozz)"};
                  const auto animationPath = std::filesystem::path{
                      R"(C:\Users\Matt\Projects\game-assets\models\gltf-working\animation.ozz)"};
                  // skinnedModelFutures.push_back(
                  //     gameplayFacade.loadSkinnedModelAsync(modelName, skeletonPath,
                  //     animationPath));
               }

               ImGui::EndGroup();
            }
         }
         ImGui::End();
         */
      }

      void renderAnimationArea(tr::ctx::GameplayFacade& gameplayFacade, ) {
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
   }