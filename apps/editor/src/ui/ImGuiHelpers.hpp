#pragma once

#include "data/DataFacade.hpp"
#include <imgui.h>
#include <imgui_stdlib.h>

namespace ed::ui::helpers {

   void renderImportSkeletonModal(data::DataFacade& dataFacade) {
      constexpr auto SkeletonFilters =
          std::array<nfdfilteritem_t, 1>{nfdfilteritem_t{"Ozz Skeleton", "ozz"}};

      if (ImGui::BeginPopupModal("Import Skeleton")) {
         static std::string skeletonFilename{}; // Temporary value to store input
         static std::string skeletonName{"Unnamed Skeleton"};

         ImGui::Text("Skeleton Name");
         ImGui::InputText("##nameInput", &skeletonName);

         ImGui::Text("Skeleton File");
         ImGui::InputText("##ValueInput", &skeletonFilename);
         ImGui::SameLine();
         if (ImGui::Button("...")) {
            auto inPath = NFD::UniquePath{};
            const auto result =
                NFD::OpenDialog(inPath, SkeletonFilters.data(), SkeletonFilters.size());
            if (result == NFD_OKAY) {
               skeletonFilename = std::string{inPath.get()};
            } else if (result == NFD_CANCEL) {
               Log::info << "User pressed cancel." << std::endl;
            } else {
               Log::error << "Error: " << NFD::GetError() << std::endl;
            }
         }

         if (ImGui::Button("OK", ImVec2(120, 0))) {
            dataFacade.addSkeleton(skeletonName, skeletonFilename);
            skeletonName = "";
            skeletonFilename = "";
            ImGui::CloseCurrentPopup();
         }
         ImGui::SameLine();
         if (ImGui::Button("Cancel", ImVec2(120, 0))) {
            ImGui::CloseCurrentPopup();
         }

         ImGui::EndPopup();
      }
   }

   void renderImportAnimationModal(data::DataFacade& dataFacade) {
      constexpr auto AnimationFilters =
          std::array<nfdfilteritem_t, 1>{nfdfilteritem_t{"Ozz Animation", "ozz"}};

      if (ImGui::BeginPopupModal("Import Animation")) {
         static std::string filename{}; // Temporary value to store input
         static std::string name{"Unnamed Animation"};

         ImGui::Text("Animation Name");
         ImGui::InputText("##nameInput", &name);

         ImGui::Text("Animation File");
         ImGui::InputText("##ValueInput", &filename);
         ImGui::SameLine();
         if (ImGui::Button("...")) {
            auto inPath = NFD::UniquePath{};
            const auto result =
                NFD::OpenDialog(inPath, AnimationFilters.data(), AnimationFilters.size());
            if (result == NFD_OKAY) {
               filename = std::string{inPath.get()};
            } else if (result == NFD_CANCEL) {
               Log::info << "User pressed cancel." << std::endl;
            } else {
               Log::error << "Error: " << NFD::GetError() << std::endl;
            }
         }

         if (ImGui::Button("OK", ImVec2(120, 0))) {
            dataFacade.addAnimation(name, filename);
            name = "";
            filename = "";
            ImGui::CloseCurrentPopup();
         }
         ImGui::SameLine();
         if (ImGui::Button("Cancel", ImVec2(120, 0))) {
            ImGui::CloseCurrentPopup();
         }

         ImGui::EndPopup();
      }
   }

   void renderAssetTree(data::DataFacade& dataFacade,
                        bool& openSkeleton,
                        bool& openAnimation,
                        bool& openModel) {
      const auto unsaved = dataFacade.isUnsaved() ? ImGuiWindowFlags_UnsavedDocument : 0;
      if (ImGui::Begin("Asset Tree", nullptr, ImGuiWindowFlags_MenuBar | unsaved)) {

         if (ImGui::BeginMenuBar()) {
            if (ImGui::BeginMenu("Import")) {
               if (ImGui::MenuItem("Skeleton...")) {
                  openSkeleton = true;
               }
               if (ImGui::MenuItem("Animation...")) {
                  openAnimation = true;
               }
               if (ImGui::MenuItem("Model...")) {
                  openModel = true;
               }
               ImGui::EndMenu();
            }
            ImGui::EndMenuBar();
         }

         static auto headerState = std::array<bool, 4>{true, true, true, true};

         {
            auto ss = std::stringstream{};
            ss << "Scripts (" << 3 << ")";
            const auto str = ss.str();
            ImGui::SetNextItemOpen(headerState[0]);
            if (ImGui::CollapsingHeader(str.c_str())) {
               ImGui::Selectable("MainCharacter");
               ImGui::Selectable("MillAbout");
               ImGui::Selectable("IdleInPlace");
               headerState[0] = true;
            } else {
               headerState[0] = false;
            }
         }

         {
            auto ss = std::stringstream{};
            ss << "Skeletons (" << dataFacade.getSkeletons().size() << ")";
            const auto str = ss.str();
            ImGui::SetNextItemOpen(headerState[1]);
            if (ImGui::CollapsingHeader(str.c_str())) {
               for (const auto& [name, filename] : dataFacade.getSkeletons()) {
                  ImGui::Selectable(name.c_str());
               }
               headerState[1] = true;
            } else {
               headerState[1] = false;
            }
         }

         {
            auto ss = std::stringstream{};
            ss << "Animations (" << dataFacade.getAnimations().size() << ")";
            const auto str = ss.str();
            ImGui::SetNextItemOpen(headerState[2]);
            if (ImGui::CollapsingHeader(str.c_str())) {
               for (const auto& [name, filename] : dataFacade.getAnimations()) {
                  ImGui::Selectable(name.c_str());
               }
               headerState[2] = true;
            } else {
               headerState[2] = false;
            }
         }

         {
            auto ss = std::stringstream{};
            ss << "Models (" << dataFacade.getModels().size() << ")";
            const auto str = ss.str();
            ImGui::SetNextItemOpen(headerState[3]);
            if (ImGui::CollapsingHeader(str.c_str())) {
               for (const auto& [name, filename] : dataFacade.getModels()) {
                  ImGui::Selectable(name.c_str());
               }
               headerState[3] = true;
            } else {
               headerState[3] = false;
            }
         }
      }
      ImGui::End();
   }
}