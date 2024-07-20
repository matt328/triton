#include "Manager.hpp"

#include "tr/GameplayFacade.hpp"

#include "Properties.hpp"
#include "RobotoRegular.h"
#include "SourceCodePro.h"

#include "data/DataFacade.hpp"
#include "ImGuiHelpers.hpp"
#include "components/AppLog.hpp"
#include "ImGuiSink.hpp"

namespace ed::ui {
   Manager::Manager(tr::ctx::GameplayFacade& facade, data::DataFacade& dataFacade)
       : facade{facade}, dataFacade{dataFacade} {
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

      sauce = fontAtlas->AddFontFromMemoryTTF(SourceCodePro_Regular_ttf,
                                              SourceCodePro_Regular_ttf_len,
                                              18.f,
                                              &config,
                                              ranges);

      ImGui_ImplVulkan_CreateFontsTexture();

      guard = std::make_unique<NFD::Guard>();

      appLog = std::make_unique<ui::cmp::AppLog>();

      const auto logFn = [this](std::string message) { appLog->AddLog("%s", message.c_str()); };

      Log.sinks().push_back(std::make_shared<my_sink_mt>(logFn));
   }

   Manager::~Manager() {
      Log.sinks().clear();
   }

   void Manager::render() {
      ZoneNamedN(guiRender, "Gui Render", true);

      dataFacade.update();

      handleSkinnedModelFutures();

      if (wireframeCallback) {
         wireframeCallback(enableWireframe);
      }

      renderDockSpace();
      renderMenuBar();
      TracyMessageL("After renderMenuBar");
      entityEditor.render(facade, dataFacade);

      renderDebugWindow();

      appLog->font = sauce;
      appLog->Draw("Log");

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

   void Manager::handleSkinnedModelFutures() {
      for (auto it = skinnedModelFutures.begin(); it != skinnedModelFutures.end();) {
         auto status = it->wait_for(std::chrono::seconds(0));
         if (status == std::future_status::ready) {
            ZoneNamedN(loadComplete, "Creating Mesh Entities", true);
            try {
               auto r = it->get();
               facade.createSkinnedModelEntity(r);
            } catch (const std::exception& e) { Log.error("Error loading model: {0}"); }
            it = skinnedModelFutures.erase(it);
         } else {
            ++it;
         }
      }
   }

   void Manager::renderDockSpace() {
      static ImGuiDockNodeFlags dockspaceFlags = ImGuiDockNodeFlags_PassthruCentralNode;
      ImGuiWindowFlags windowFlags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;

      ImGuiViewport* viewport = ImGui::GetMainViewport();
      ImGui::SetNextWindowPos(viewport->Pos);
      ImGui::SetNextWindowSize(viewport->Size);
      ImGui::SetNextWindowViewport(viewport->ID);
      ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
      ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
      windowFlags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
                     ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
      windowFlags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

      if (dockspaceFlags & ImGuiDockNodeFlags_PassthruCentralNode)
         windowFlags |= ImGuiWindowFlags_NoBackground;

      ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
      ImGui::Begin("DockSpace", nullptr, windowFlags);
      ImGui::PopStyleVar();
      ImGui::PopStyleVar(2);

      // DockSpace
      ImGuiIO& io = ImGui::GetIO();
      if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
         ImGuiID dockspaceId = ImGui::GetID("MyDockSpace");
         ImGui::DockSpace(dockspaceId, ImVec2(0.0f, 0.0f), dockspaceFlags);

         static auto first_time = true;
         if (first_time) {
            first_time = false;

            ImGui::DockBuilderRemoveNode(dockspaceId); // clear any previous layout
            ImGui::DockBuilderAddNode(dockspaceId, dockspaceFlags | ImGuiDockNodeFlags_DockSpace);
            ImGui::DockBuilderSetNodeSize(dockspaceId, viewport->Size);

            auto dockIdLeft = ImGui::DockBuilderSplitNode(dockspaceId,
                                                          ImGuiDir_Left,
                                                          0.2f,
                                                          nullptr,
                                                          &dockspaceId);

            auto dockIdRight = ImGui::DockBuilderSplitNode(dockspaceId,
                                                           ImGuiDir_Down,
                                                           0.2f,
                                                           nullptr,
                                                           &dockspaceId);

            ImGui::DockBuilderDockWindow("Entity Editor", dockIdLeft);
            ImGui::DockBuilderDockWindow("Asset Tree", dockIdLeft);
            ImGui::DockBuilderDockWindow("Log", dockIdRight);

            ImGui::DockBuilderFinish(dockspaceId);
         }
      }
      ImGui::End(); // Dockspace
   }

   void Manager::renderMenuBar() {
      auto b = false;
      auto showPopup = false;
      auto showAnimation = false;
      auto loadTerrain = false;
      static auto show = true;

      if (ImGui::BeginMainMenuBar()) {
         if (ImGui::BeginMenu("File")) {

            if (ImGui::MenuItem("Create Terrain")) {
               loadTerrain = true;
            }

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
                  Log.error("File Dialog Error: ", NFD::GetError());
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
                  } catch (const std::exception& ex) { Log.error(ex.what()); }
               }
            }

            if (ImGui::MenuItem("Save Project As...", nullptr, false, dataFacade.isUnsaved())) {
               auto savePath = getSavePath();
               try {
                  if (savePath.has_value()) {
                     dataFacade.save(savePath.value());
                  }
               } catch (const std::exception& ex) { Log.error(ex.what()); }
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
            Log.info("Ok");
         }
         ImGui::SetItemDefaultFocus();
         ImGui::SameLine();
         if (ImGui::Button("Cancel", ImVec2(120, 0))) {
            ImGui::CloseCurrentPopup();
         }
         ImGui::EndPopup();
      }
      ImGui::ShowDemoWindow(&show);

      if (loadTerrain) {
         ZoneNamedN(n, "Create Terrain", true);
         TracyMessageL("Before CreateTerrain");
         terrainFutures.push_back(facade.createTerrain(1024));
      }
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
         Log.debug("Success: {0}", outPath.get());
         return std::optional{std::filesystem::path{outPath.get()}};
      } else if (result == NFD_CANCEL) {
         Log.debug("User pressed Cancel");
         return std::nullopt;
      } else {
         Log.error("Error getting save path: {0}", NFD::GetError());
         return std::nullopt;
      }
   }
}