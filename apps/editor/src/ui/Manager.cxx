#include "Manager.hpp"
#include "ctx/GameplayFacade.hpp"
#include "Properties.hpp"
#include "RobotoRegular.h"
#include "data/DataFacade.hpp"
#include "ImGuiHelpers.hpp"
#include "ui/components/EntityEditor.hpp"
#include <imgui.h>

/*
   TODO: view showing skeletons, animations, models in the current project.
*/

namespace ed::ui {
   Manager::Manager(tr::ctx::GameplayFacade& facade, data::DataFacade& dataFacade)
       : facade{facade},
         dataFacade{dataFacade},
         entityEditor{components::EntityEditor{[this, &facade](uint32_t size) {
            terrainFutures.push_back(facade.createTerrainMesh(512));
         }}} {
      ImGuiEx::setupImGuiStyle();
      ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;

      auto& io = ImGui::GetIO();
      auto fontAtlas = io.Fonts;
      const auto ranges = io.Fonts->GetGlyphRangesDefault();

      auto config = ImFontConfig{};
      config.FontDataOwnedByAtlas = false;
      fontAtlas->AddFontFromMemoryTTF(Roboto_Regular_ttf,
                                      Roboto_Regular_ttf_len,
                                      18.f,
                                      &config,
                                      ranges);

      ImGui_ImplVulkan_CreateFontsTexture();

      guard = std::make_unique<NFD::Guard>();
   }

   Manager::~Manager() {
   }

   void Manager::render() {
      ZoneNamedN(guiRender, "Gui Render", true);
      handleTerrainFutures();
      handleModelFutures();
      handleSkinnedModelFutures();

      if (wireframeCallback) {
         wireframeCallback(enableWireframe);
      }

      renderDockSpace();
      renderMenuBar();
      entityEditor.render(facade, dataFacade);
      renderDebugWindow();

      helpers::renderImportSkeletonModal(dataFacade);
      helpers::renderImportAnimationModal(dataFacade);
      helpers::renderImportModelModal(dataFacade);

      bool openSkeleton{};
      bool openAnimation{};
      bool openModel{};

      helpers::renderAssetTree(dataFacade, openSkeleton, openAnimation, openModel);

      if (openSkeleton) {
         ImGui::OpenPopup("Import Skeleton");
      }

      if (openAnimation) {
         ImGui::OpenPopup("Import Animation");
      }

      if (openModel) {
         ImGui::OpenPopup("Import Model");
      }
   }

   /*
      Can this be moved inside the facade, and just register a callback to be called after it's all
      done?
      so like
      facade.doAsyncThing([]() {Log::debug << "done" << std::endl;})
      Context would have to call gameplayFacade.tick() or something so it could check it's internal
      list of futures
   */
   void Manager::handleTerrainFutures() {
      for (auto it = terrainFutures.begin(); it != terrainFutures.end();) {
         auto status = it->wait_for(std::chrono::seconds(0));
         if (status == std::future_status::ready) {
            ZoneNamedN(loadComplete, "Creating Terrain Entities", true);
            try {
               auto r = it->get();
               facade.createTerrainEntity(r);
            } catch (const std::exception& e) {
               Log::error << "error loading model: " << e.what() << std::endl;
            }
            it = terrainFutures.erase(it);
         } else {
            ++it;
         }
      }
   }

   void Manager::handleModelFutures() {
      for (auto it = modelFutures.begin(); it != modelFutures.end();) {
         auto status = it->wait_for(std::chrono::seconds(0));
         if (status == std::future_status::ready) {
            ZoneNamedN(loadComplete, "Creating Mesh Entities", true);
            try {
               auto r = it->get();
               facade.createStaticMultiMeshEntity(r);
            } catch (const std::exception& e) {
               Log::error << "error loading model: " << e.what() << std::endl;
            }
            it = modelFutures.erase(it);
         } else {
            ++it;
         }
      }
   }

   void Manager::handleSkinnedModelFutures() {
      for (auto it = skinnedModelFutures.begin(); it != skinnedModelFutures.end();) {
         auto status = it->wait_for(std::chrono::seconds(0));
         if (status == std::future_status::ready) {
            ZoneNamedN(loadComplete, "Creating Mesh Entities", true);
            try {
               auto r = it->get();
               facade.createSkinnedModelEntity(r);
            } catch (const std::exception& e) {
               Log::error << "error loading model: " << e.what() << std::endl;
            }
            it = skinnedModelFutures.erase(it);
         } else {
            ++it;
         }
      }
   }

   void Manager::renderDockSpace() {
      static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_PassthruCentralNode;
      ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;

      ImGuiViewport* viewport = ImGui::GetMainViewport();
      ImGui::SetNextWindowPos(viewport->Pos);
      ImGui::SetNextWindowSize(viewport->Size);
      ImGui::SetNextWindowViewport(viewport->ID);
      ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
      ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
      window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
                      ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
      window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

      if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
         window_flags |= ImGuiWindowFlags_NoBackground;

      ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
      ImGui::Begin("DockSpace", nullptr, window_flags);
      ImGui::PopStyleVar();
      ImGui::PopStyleVar(2);

      // DockSpace
      ImGuiIO& io = ImGui::GetIO();
      if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
         ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
         ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);

         static auto first_time = true;
         if (first_time) {
            first_time = false;

            ImGui::DockBuilderRemoveNode(dockspace_id); // clear any previous layout
            ImGui::DockBuilderAddNode(dockspace_id, dockspace_flags | ImGuiDockNodeFlags_DockSpace);
            ImGui::DockBuilderSetNodeSize(dockspace_id, viewport->Size);
            auto dock_id_left = ImGui::DockBuilderSplitNode(dockspace_id,
                                                            ImGuiDir_Left,
                                                            0.2f,
                                                            nullptr,
                                                            &dockspace_id);
            ImGui::DockBuilderDockWindow("Entity Editor", dock_id_left);
            ImGui::DockBuilderDockWindow("Asset Tree", dock_id_left);
            ImGui::DockBuilderFinish(dockspace_id);
         }
      }
      ImGui::End(); // Dockspace
   }

   void Manager::renderMenuBar() {
      auto b = false;
      auto showPopup = false;
      auto showAnimation = false;
      static auto show = true;

      if (ImGui::BeginMainMenuBar()) {
         if (ImGui::BeginMenu("File")) {

            if (ImGui::MenuItem("Import Skeleton")) {
               showPopup = true;
            }

            if (ImGui::MenuItem("Import Animation")) {
               showAnimation = true;
            }

            if (ImGui::MenuItem("New Project...")) {
               if (dataFacade.isUnsaved()) {
                  b = true;
               }
            }

            ImGui::Separator();
            if (ImGui::MenuItem("Open Project...")) {
               auto inPath = NFD::UniquePath{};
               const auto result =
                   NFD::OpenDialog(inPath, ProjectFileFilters.data(), ProjectFileFilters.size());
               if (result == NFD_OKAY) {
                  const auto filePath = std::filesystem::path{inPath.get()};
                  dataFacade.load(filePath);
                  openFilePath.emplace(filePath);
                  pr::Properties::getInstance().setRecentFilePath(filePath);
               } else {
                  Log::error << "Error: " << NFD::GetError() << std::endl;
               }
            }

            if (ImGui::BeginMenu("Open Recent")) {
               const auto recentFile = pr::Properties::getInstance().getRecentFilePath();
               if (recentFile.has_value()) {
                  const auto nameOnly = recentFile.value().string();
                  if (ImGui::MenuItem(nameOnly.c_str())) {
                     dataFacade.clear();
                     dataFacade.load(recentFile.value());
                     openFilePath.emplace(recentFile.value());
                  }
               }
               ImGui::EndMenu();
            }

            ImGui::Separator();
            if (ImGui::MenuItem("Save Project", "Ctrl+S", false, dataFacade.isUnsaved())) {
               if (openFilePath.has_value()) {
                  dataFacade.save(openFilePath.value());
               } else {
                  auto savePath = getSavePath();
                  try {
                     if (savePath.has_value()) {
                        dataFacade.save(savePath.value());
                     }
                  } catch (const std::exception& ex) { Log::error << ex.what() << std::endl; }
               }
            }

            if (ImGui::MenuItem("Save Project As...", nullptr, false, dataFacade.isUnsaved())) {
               auto savePath = getSavePath();
               try {
                  if (savePath.has_value()) {
                     dataFacade.save(savePath.value());
                  }
               } catch (const std::exception& ex) { Log::error << ex.what() << std::endl; }
            }

            ImGui::Separator();

            if (ImGui::MenuItem("Exit", "Alt+F4")) {
               quitDelegate();
            }
            ImGui::EndMenu();
         }
         if (ImGui::BeginMenu("View")) {
            if (ImGui::MenuItem("Fullscreen", "Alt+Enter", this->fullscreen)) {
               this->fullscreen = !this->fullscreen;
               toggleFullscreenFn();
            }
            if (ImGui::MenuItem("Demo Window", nullptr)) {
               show = !show;
            }
            ImGui::EndMenu();
         }
         ImGui::EndMainMenuBar();
      }

      if (showPopup) {
         ImGui::OpenPopup("Import Skeleton");
      }

      if (showAnimation) {
         ImGui::OpenPopup("Import Animation");
      }

      if (b) {
         ImGui::OpenPopup("Unsaved");
      }

      if (ImGui::BeginPopupModal("Unsaved", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
         ImGui::Text("Unsaved changes will be lost. Are you sure?");
         ImGui::Separator();

         if (ImGui::Button("Ok", ImVec2(120, 0))) {
            ImGui::CloseCurrentPopup();
            Log::info << "ok" << std::endl;
         }
         ImGui::SetItemDefaultFocus();
         ImGui::SameLine();
         if (ImGui::Button("Cancel", ImVec2(120, 0))) {
            ImGui::CloseCurrentPopup();
         }
         ImGui::EndPopup();
      }
      ImGui::ShowDemoWindow(&show);
   }

   void Manager::showMatrix4x4(const glm::mat4& matrix, const char* label) {
      ImGui::Text("%s", label);
      for (int col = 0; col < 4; ++col) {
         ImGui::Text("%7.3f %7.3f %7.3f %7.3f",
                     matrix[0][col],
                     matrix[1][col],
                     matrix[2][col],
                     matrix[3][col]);
      }
   }

   void Manager::renderDebugWindow() {
      ImGui::Begin("Debug Window");

      ImGui::Checkbox("Wireframe", &enableWireframe);

      ImGui::End();
   }

   auto Manager::getSavePath() -> std::optional<std::filesystem::path> {
      auto outPath = NFD::UniquePath{};

      const auto result =
          NFD::SaveDialog(outPath, ProjectFileFilters.data(), ProjectFileFilters.size());
      if (result == NFD_OKAY) {
         Log::info << "Success: " << outPath.get() << std::endl;
         return std::optional{std::filesystem::path{outPath.get()}};
      } else if (result == NFD_CANCEL) {
         Log::info << "User pressed cancel." << std::endl;
         return std::nullopt;
      } else {
         Log::error << "Error: " << NFD::GetError() << std::endl;
         return std::nullopt;
      }
   }
}