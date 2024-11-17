#include "AssetViewer.hpp"
#include "cm/GlmToString.hpp"

namespace ed::ui::cmp {
   AssetViewer::AssetViewer(std::shared_ptr<data::DataFacade> newDataFacade,
                            std::shared_ptr<DialogManager> newDialogManager)
       : dataFacade{std::move(newDataFacade)}, dialogManager{std::move(newDialogManager)} {
      Log.trace("Constructing AssetViewer");
      createSkeletonDialog();
      createAnimationDialog();
      createModelDialog();
   }

   AssetViewer::~AssetViewer() {
      Log.trace("Destroying AssetViewer");
   }

   void AssetViewer::render() {
      if (const auto unsaved = dataFacade->isUnsaved() ? ImGuiWindowFlags_UnsavedDocument : 0;
          ImGui::Begin("Asset Tree", nullptr, ImGuiWindowFlags_MenuBar | unsaved)) {

         if (ImGui::BeginMenuBar()) {
            if (ImGui::BeginMenu("Import")) {
               if (ImGui::MenuItem("Skeleton...")) {
                  dialogManager->setOpen("Skeleton");
               }
               if (ImGui::MenuItem("Animation...")) {
                  dialogManager->setOpen("Animation");
               }
               if (ImGui::MenuItem("Model...")) {
                  dialogManager->setOpen("Model");
               }
               ImGui::EndMenu();
            }
            ImGui::EndMenuBar();
         }

         static auto headerState = std::array{true, true, true, true};

         {
            ImGui::SetNextItemOpen(headerState[0]);
            if (ImGui::CollapsingHeader("Scripts")) {
               ImGui::Selectable("MainCharacter");
               ImGui::Selectable("MillAbout");
               ImGui::Selectable("IdleInPlace");
               headerState[0] = true;
            } else {
               headerState[0] = false;
            }
         }

         {
            ImGui::SetNextItemOpen(headerState[1]);
            if (ImGui::CollapsingHeader("Skeletons")) {
               for (const auto& name : dataFacade->getSkeletons() | std::views::keys) {
                  ImGui::Selectable(name.c_str());
               }
               headerState[1] = true;
            } else {
               headerState[1] = false;
            }
         }

         {
            ImGui::SetNextItemOpen(headerState[2]);
            if (ImGui::CollapsingHeader("Animations")) {
               for (const auto& name : dataFacade->getAnimations() | std::views::keys) {
                  ImGui::Selectable(name.c_str());
               }
               headerState[2] = true;
            } else {
               headerState[2] = false;
            }
         }

         {
            ImGui::SetNextItemOpen(headerState[3]);
            if (ImGui::CollapsingHeader("Models")) {
               for (const auto& name : dataFacade->getModels() | std::views::keys) {
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

   void AssetViewer::createSkeletonDialog() {
      auto dialog = std::make_unique<cmp::ModalDialog>(
          "Skeleton",
          [&](const cmp::ModalDialog& dialog) {
             Log.trace("name: {0}, file: {1}",
                       dialog.getValue<std::string>("name").value(),
                       dialog.getValue<std::filesystem::path>("filename").value().string());
             dataFacade->addSkeleton(
                 dialog.getValue<std::string>("name").value(),
                 dialog.getValue<std::filesystem::path>("filename").value().string());
          },
          []() { Log.debug("Cancelled Dialog with no input"); });

      dialog->addControl("name", "Skeleton Name", std::string("Unnamed Skeleton"));
      dialog->addControl("filename", "Skeleton File", std::filesystem::path{});

      dialogManager->addDialog("Skeleton", std::move(dialog));
   }

   void AssetViewer::createAnimationDialog() {
      auto dialog = std::make_unique<cmp::ModalDialog>(
          "Animation",
          [&](const cmp::ModalDialog& dialog) {
             Log.trace("name: {0}, file: {1}",
                       dialog.getValue<std::string>("name").value(),
                       dialog.getValue<std::filesystem::path>("filename").value().string());
             dataFacade->addAnimation(
                 dialog.getValue<std::string>("name").value(),
                 dialog.getValue<std::filesystem::path>("filename").value().string());
          },
          []() { Log.debug("Cancelled Dialog with no input"); });

      dialog->addControl("name", "Animation Name", std::string("Unnamed Animation"));
      dialog->addControl("filename", "Animation File", std::filesystem::path{});

      dialogManager->addDialog("Animation", std::move(dialog));
   }

   void AssetViewer::createModelDialog() {
      auto dialog = std::make_unique<cmp::ModalDialog>(
          "Model",
          [&](const cmp::ModalDialog& dialog) {
             Log.trace("name: {0}, file: {1}",
                       dialog.getValue<std::string>("name").value(),
                       dialog.getValue<std::filesystem::path>("filename").value().string());
             dataFacade->addModel(
                 dialog.getValue<std::string>("name").value(),
                 dialog.getValue<std::filesystem::path>("filename").value().string());
          },
          []() { Log.debug("Cancelled Dialog with no input"); });

      dialog->addControl("name", "Model Name", std::string("Unnamed Model"));
      dialog->addControl("filename", "Model File", std::filesystem::path{});

      dialogManager->addDialog("Model", std::move(dialog));
   }
}