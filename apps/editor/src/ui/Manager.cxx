#include "Manager.hpp"
#include "ctx/GameplayFacade.hpp"
#include "Properties.hpp"
#include "RobotoRegular.h"
#include "data/DataFacade.hpp"
#include "gp/ecs/component/DebugConstants.hpp"
#include "ImGuiHelpers.hpp"

/*
   TODO: view showing skeletons, animations, models in the current project.
*/

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
      renderEntityEditor();
      renderDebugWindow();
      helpers::renderAssetTree(dataFacade);
   }

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

      helpers::renderImportSkeletonModal(dataFacade);
      helpers::renderImportAnimationModal(dataFacade);

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

   void Manager::renderEntityEditor() {
      // Entity Editor
      auto& es = facade.getAllEntities();

      if (ImGui::Begin("Entity Editor",
                       nullptr,
                       dataFacade.isUnsaved() ? ImGuiWindowFlags_UnsavedDocument : 0)) {
         // Left
         ImGui::BeginChild("left pane",
                           ImVec2(150, 0),
                           ImGuiChildFlags_Border | ImGuiChildFlags_ResizeX);
         for (auto e : es) {
            auto& infoComponent = facade.getEditorInfo(e);
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
            ImGui::BeginChild(
                "item view",
                ImVec2(0,
                       -ImGui::GetFrameHeightWithSpacing() * 4)); // Leave room for 1 line below us

            if (selectedEntity.has_value()) {
               ImGui::Text("Entity ID: %d", selectedEntity.value());
            } else {
               ImGui::Text("No Entity Selected");
            }

            if (selectedEntity.has_value()) {
               const auto maybeTransform = facade.getComponent<tr::gp::ecs::Transform>(
                   static_cast<entt::entity>(selectedEntity.value()));
               const auto maybeDebug = facade.getComponent<tr::gp::ecs::DebugConstants>(
                   static_cast<entt::entity>(selectedEntity.value()));
               const auto maybeAnimated = facade.getComponent<tr::gp::ecs::Animation>(
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
                  renderAnimationArea(animationComponent);
               }
            }
            ImGui::EndChild();

            if (ImGui::Button("Load Terrain")) {
               terrainFutures.push_back(facade.createTerrainMesh(512));
            }
            ImGui::SameLine();
            if (ImGui::Button("Load Cube")) {
               const auto modelName = std::filesystem::path{
                   R"(C:\Users\Matt\Projects\game-assets\models\cube\cube2.gltf)"};
               const auto skeletonPath = std::filesystem::path{
                   R"(C:\Users\Matt\Projects\game-assets\models\cube\skeleton.ozz)"};
               const auto animationPath = std::filesystem::path{
                   R"(C:\Users\Matt\Projects\game-assets\models\cube\animation.ozz)"};
               skinnedModelFutures.push_back(
                   facade.loadSkinnedModelAsync(modelName, skeletonPath, animationPath));
            }
            if (ImGui::Button("Load Cesium")) {
               const auto modelName = std::filesystem::path{
                   R"(C:\Users\Matt\Projects\game-assets\models\cesiumman\CesiumMan.gltf)"};
               const auto skeletonPath = std::filesystem::path{
                   R"(C:\Users\Matt\Projects\game-assets\models\cesiumman\skeleton.ozz)"};
               const auto animationPath = std::filesystem::path{
                   R"(C:\Users\Matt\Projects\game-assets\models\cesiumman\animation.ozz)"};
               skinnedModelFutures.push_back(
                   facade.loadSkinnedModelAsync(modelName, skeletonPath, animationPath));
            }

            if (ImGui::Button("Load Cesium Reexport")) {
               const auto modelName = std::filesystem::path{
                   R"(C:\Users\Matt\Projects\game-assets\models\cesiumman\reexport\CesiumManReexported.gltf)"};
               const auto skeletonPath = std::filesystem::path{
                   R"(C:\Users\Matt\Projects\game-assets\models\cesiumman\reexport\skeleton.ozz)"};
               const auto animationPath = std::filesystem::path{
                   R"(C:\Users\Matt\Projects\game-assets\models\cesiumman\reexport\animation.ozz)"};
               skinnedModelFutures.push_back(
                   facade.loadSkinnedModelAsync(modelName, skeletonPath, animationPath));
            }

            if (ImGui::Button("Load Peasant")) {
               const auto modelName = std::filesystem::path{
                   R"(C:\Users\Matt\Projects\game-assets\models\gltf-working\walking.gltf)"};
               const auto skeletonPath = std::filesystem::path{
                   R"(C:\Users\Matt\Projects\game-assets\models\gltf-working\skeleton.ozz)"};
               const auto animationPath = std::filesystem::path{
                   R"(C:\Users\Matt\Projects\game-assets\models\gltf-working\animation.ozz)"};
               skinnedModelFutures.push_back(
                   facade.loadSkinnedModelAsync(modelName, skeletonPath, animationPath));
            }

            ImGui::EndGroup();
         }
      }
      ImGui::End();
   }

   void Manager::renderAnimationArea(tr::gp::ecs::Animation& animationComponent) {
      ImGui::SeparatorText("Animation");
      static size_t itemCurrentIndex{};
      static bool playing{};
      static bool bindPose{};

      std::vector<const char*> items;
      items.push_back("Animation");

      if (ImGui::BeginCombo("Animation Name", items[itemCurrentIndex])) {
         for (size_t n = 0; n < items.size(); ++n) {
            const bool isSelected = (itemCurrentIndex == n);
            if (ImGui::Selectable(items[n], isSelected)) {
               itemCurrentIndex = n;
               animationComponent.currentAnimationName = std::string{items[itemCurrentIndex]};
            }
            if (isSelected) {
               ImGui::SetItemDefaultFocus();
            }
         }
         ImGui::EndCombo();
      }

      bool previousBindPose = bindPose;

      ImGui::BeginDisabled(playing);
      if (ImGui::Checkbox("Bind Pose", &bindPose)) {
         if (bindPose != previousBindPose) {
            animationComponent.renderBindPose = bindPose;
         }
      }
      ImGui::EndDisabled();

      bool previousPlaying = playing;

      ImGui::BeginDisabled(bindPose);

      if (ImGui::Checkbox("Play", &playing)) {
         if (playing != previousPlaying) {
            animationComponent.playing = playing;
         }
      }

      ImGui::BeginDisabled(playing);
      const auto [min_v, max_v] = facade.getAnimationTimeRange(animationComponent.animationHandle);

      static float timeValue = min_v;
      if (ImGui::SliderFloat("Time", &timeValue, min_v, max_v, "%.2f")) {
         const auto value = (timeValue - min_v) / (max_v - min_v);
         animationComponent.timeRatio = value;
      }
      ImGui::EndDisabled();
      ImGui::EndDisabled();
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