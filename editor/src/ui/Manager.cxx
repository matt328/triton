#include "Manager.hpp"
#include "ctx/GameplayFacade.hpp"
#include "Properties.hpp"
#include "ProjectFile.hpp"
#include "RobotoRegular.h"

namespace ed::ui {
   Manager::Manager(tr::ctx::GameplayFacade& facade) : facade{facade} {
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

      openProjectFileDialog.SetTitle("Load Project");
      openProjectFileDialog.SetTypeFilters({".json"});

      const auto recentPath = pr::Properties::getInstance().getRecentFilePath();

      if (recentPath.has_value()) {
         openProjectFileDialog.SetPwd(recentPath.value().parent_path());
         saveProjectFileDialog.SetPwd(recentPath.value().parent_path());
      }

      saveProjectFileDialog.SetTitle("Save Project");
      saveProjectFileDialog.SetTypeFilters({".json"});
   }

   Manager::~Manager() {
   }

   void Manager::render() {
      ZoneNamedN(guiRender, "Gui Render", true);
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
      renderDockSpace();
      renderMenuBar();
      renderEntityEditor();
      renderDialogs();
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
            ImGui::DockBuilderFinish(dockspace_id);
         }
      }
      ImGui::End(); // Dockspace
   }

   void Manager::renderMenuBar() {
      auto b = false;
      if (ImGui::BeginMainMenuBar()) {
         if (ImGui::BeginMenu("File")) {

            if (ImGui::MenuItem("New Project...")) {
               if (dirty) {
                  b = true;
               }
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Open Project...")) {
               openProjectFileDialog.Open();
            }

            if (ImGui::BeginMenu("Open Recent")) {
               const auto recentFile = pr::Properties::getInstance().getRecentFilePath();
               if (recentFile.has_value()) {
                  const auto nameOnly = recentFile.value().string();
                  if (ImGui::MenuItem(nameOnly.c_str())) {
                     facade.clear();
                     io::readProjectFile(recentFile.value().string(), facade);
                     openFilePath.emplace(recentFile.value());
                  }
               }
               ImGui::EndMenu();
            }

            ImGui::Separator();
            if (ImGui::MenuItem("Save Project", nullptr, false, dirty)) {
               if (openFilePath.has_value()) {
                  io::writeProjectFile(openFilePath.value().string(), facade);
                  dirty = false;
               } else {
                  saveProjectFileDialog.Open();
               }
            }

            if (ImGui::MenuItem("Save Project As...", nullptr, false, dirty)) {
               saveProjectFileDialog.Open();
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
            ImGui::EndMenu();
         }
         ImGui::EndMainMenuBar();
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
   }

   void Manager::renderEntityEditor() {
      // Entity Editor
      auto& es = facade.getAllEntities();

      if (ImGui::Begin("Entity Editor", nullptr, dirty ? ImGuiWindowFlags_UnsavedDocument : 0)) {
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
                       -ImGui::GetFrameHeightWithSpacing())); // Leave room for 1 line below us

            if (selectedEntity.has_value()) {
               ImGui::Text("Entity ID: %d", selectedEntity.value());
            } else {
               ImGui::Text("No Entity Selected");
            }

            if (selectedEntity.has_value()) {
               const auto maybeTransform = facade.getComponent<tr::gp::ecs::Transform>(
                   static_cast<entt::entity>(selectedEntity.value()));
               if (maybeTransform.has_value()) {
                  auto& transform = maybeTransform.value().get();
                  ImGui::SeparatorText("Transform");
                  ImGui::DragFloat3("Position", glm::value_ptr(transform.position), .1f);
                  ImGui::DragFloat3("Rotation",
                                    glm::value_ptr(transform.rotation),
                                    .1f,
                                    -180.f,
                                    180.f);
                  ImGui::SeparatorText("Renderable");
               }
            }
            ImGui::EndChild();

            if (ImGui::Button("New...")) {
               openProjectFileDialog.Open();
            }
            ImGui::SameLine();
            if (ImGui::Button("Test")) {

               auto filename = std::filesystem::path{
                   R"(C:\Users\Matt\Projects\game-assets\models\Sponza\glTF\Sponza.gltf)"};

               modelFutures.push_back(facade.loadModelAsync(filename));
            }

            ImGui::EndGroup();
         }
      }
      if (ImGui::IsItemDeactivatedAfterEdit()) {
         dirty = true;
      }
      ImGui::End();
   }

   void Manager::renderDialogs() {
      openProjectFileDialog.Display();

      if (openProjectFileDialog.HasSelected()) {
         Log::info << "Selected filename " << openProjectFileDialog.GetSelected().string()
                   << std::endl;

         auto path = std::filesystem::path{openProjectFileDialog.GetSelected().string()};
         openFilePath.emplace(path.parent_path());

         facade.clear();
         io::readProjectFile(openProjectFileDialog.GetSelected().string(), facade);

         pr::Properties::getInstance().setRecentFilePath(path);

         openProjectFileDialog.ClearSelected();
      }

      saveProjectFileDialog.Display();

      if (saveProjectFileDialog.HasSelected()) {
         Log::info << "Selected filename " << saveProjectFileDialog.GetSelected().string()
                   << std::endl;
         io::writeProjectFile(saveProjectFileDialog.GetSelected().string(), facade);
         dirty = false;
         saveProjectFileDialog.ClearSelected();
      }
   }
}