#include "Manager.hpp"

#include "tr/GameplayFacade.hpp"

#include "Properties.hpp"
#include "RobotoRegular.h"
#include "SourceCodePro.h"

#include "data/DataFacade.hpp"
#include "ImGuiHelpers.hpp"
#include "components/AppLog.hpp"
#include "ImGuiSink.hpp"
#include "cm/GlmToString.hpp"

// NOLINTBEGIN(cppcoreguidelines-pro-type-vararg,
// hicpp-vararg,readability-function-cognitive-complexity,hicpp-no-array-decay,hicpp-no-pointer-decay,cppcoreguidelines-pro-bounds-array-to-pointer-decay,hicpp-signed-bitwise)

namespace ed::ui {

   static constexpr float FontSize = 18.f;

   Manager::Manager(tr::ctx::GameplayFacade& facade, data::DataFacade& dataFacade)
       : facade{facade}, dataFacade{dataFacade} {
      ImGuiEx::setupImGuiStyle2();
      ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;

      const auto& io = ImGui::GetIO();
      auto* const fontAtlas = io.Fonts;
      const auto* const ranges = io.Fonts->GetGlyphRangesDefault();

      auto config = ImFontConfig{};
      config.FontDataOwnedByAtlas = false;
      fontAtlas->AddFontFromMemoryTTF(Roboto_Regular_ttf,
                                      Roboto_Regular_ttf_len,
                                      FontSize,
                                      &config,
                                      ranges);

      sauce = fontAtlas->AddFontFromMemoryTTF(SourceCodePro_Regular_ttf,
                                              SourceCodePro_Regular_ttf_len,
                                              FontSize,
                                              &config,
                                              ranges);

      if (!ImGui_ImplVulkan_CreateFontsTexture()) {
         Log.warn("Error creating Fonts Texture");
      }

      guard = std::make_unique<NFD::Guard>();

      appLog = std::make_unique<cmp::AppLog>();

      const auto logFn = [this](const std::string& message) {
         appLog->AddLog("%s", message.c_str());
      };

      Log.sinks().push_back(std::make_shared<my_sink_mt>(logFn));

      dialog = std::make_unique<cmp::ModalDialog>(
          "Test Modal",
          [](const cmp::ModalDialog& dialog) {
             Log.debug("name: {0}, age: {1}, height: {2}, vector3: {3}, file: {4}",
                       dialog.getValue<std::string>("name").value(),
                       dialog.getValue<int>("age").value(),
                       dialog.getValue<float>("height").value(),
                       dialog.getValue<glm::vec3>("vector3").value(),
                       dialog.getValue<std::filesystem::path>("filename").value().string());
          },
          []() { Log.debug("Cancelled Dialog with no input"); });

      dialog->addControl("name", "Name", std::string("Default Name"));
      dialog->addControl("age", "Age", 25);
      dialog->addControl("height", "Height", 5.9f);
      dialog->addControl("vector3", "Vector3", glm::vec3{0.f, 0.f, 0.f});
      dialog->addControl("filename", "Skeleton File", std::filesystem::path{});
   }

   Manager::~Manager() {
      Log.sinks().clear();
      ImGui_ImplVulkan_DestroyFontsTexture();
   }

   void Manager::render() {
      ZoneNamedN(guiRender, "Gui Render", true);

      dataFacade.update();

      if (wireframeCallback) {
         wireframeCallback(enableWireframe);
      }

      renderDockSpace();
      renderMenuBar();
      TracyMessageL("After renderMenuBar");
      entityEditor.render(facade, dataFacade);

      appLog->font = sauce;
      appLog->Draw("Log");

      dialog->render();

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

   void Manager::renderDockSpace() {
      static ImGuiDockNodeFlags dockspaceFlags = ImGuiDockNodeFlags_PassthruCentralNode;

      ImGuiWindowFlags windowFlags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;

      const ImGuiViewport* viewport = ImGui::GetMainViewport();
      ImGui::SetNextWindowPos(viewport->Pos);
      ImGui::SetNextWindowSize(viewport->Size);
      ImGui::SetNextWindowViewport(viewport->ID);
      ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
      ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
      windowFlags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
                     ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
      windowFlags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

      if ((dockspaceFlags & ImGuiDockNodeFlags_PassthruCentralNode) != 0) {
         windowFlags |= ImGuiWindowFlags_NoBackground;
      }

      ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
      ImGui::Begin("DockSpace", nullptr, windowFlags);
      ImGui::PopStyleVar();
      ImGui::PopStyleVar(2);

      // DockSpace
      if (const ImGuiIO& io = ImGui::GetIO(); io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
         ImGuiID dockspaceId = ImGui::GetID("MyDockSpace");
         ImGui::DockSpace(dockspaceId, ImVec2(0.0f, 0.0f), dockspaceFlags);

         static auto first_time = true;
         if (first_time) {
            first_time = false;

            ImGui::DockBuilderRemoveNode(dockspaceId); // clear any previous layout
            ImGui::DockBuilderAddNode(dockspaceId, dockspaceFlags | ImGuiDockNodeFlags_DockSpace);
            ImGui::DockBuilderSetNodeSize(dockspaceId, viewport->Size);

            const auto dockIdLeft = ImGui::DockBuilderSplitNode(dockspaceId,
                                                                ImGuiDir_Left,
                                                                0.25f,
                                                                nullptr,
                                                                &dockspaceId);

            const auto dockIdRight = ImGui::DockBuilderSplitNode(dockspaceId,
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

   void Manager::renderMenuBar() { // NOLINT(readability-function-cognitive-complexity)
      auto b = false;
      auto showPopup = false;
      auto showAnimation = false;
      static auto show = false;

      if (ImGui::BeginMainMenuBar()) {
         if (ImGui::BeginMenu("File")) {

            if (ImGui::MenuItem("Test Dialog")) {
               dialog->setOpen();
            }

            if (ImGui::MenuItem("Create Terrain")) {
               dataFacade.createTerrain("Test Terrain");
            }

            if (ImGui::MenuItem("Create Box")) {
               dataFacade.createAABB();
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
               constexpr auto filterSize = static_cast<nfdfiltersize_t>(ProjectFileFilters.size());
               if (const auto result = OpenDialog(inPath, ProjectFileFilters.data(), filterSize);
                   result == NFD_OKAY) {
                  const auto filePath = std::filesystem::path{inPath.get()};
                  dataFacade.load(filePath);
                  openFilePath = filePath;
                  Properties::getInstance().setRecentFile(filePath);
               } else {
                  Log.error("File Dialog Error: ", NFD::GetError());
               }
            }

            if (ImGui::BeginMenu("Open Recent")) {
               if (const auto recentFile = Properties::getInstance().getRecentFile();
                   recentFile.has_value()) {
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
                  const auto savePath = getSavePath();
                  try {
                     if (savePath.has_value()) {
                        dataFacade.save(savePath.value());
                     }
                  } catch (const std::exception& ex) { Log.error(ex.what()); }
               }
            }

            if (ImGui::MenuItem("Save Project As...", nullptr, false, dataFacade.isUnsaved())) {
               const auto savePath = getSavePath();
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
            if (ImGui::MenuItem("Demo Window", nullptr, show)) {
               show = !show;
            }
            if (ImGui::MenuItem("Wireframe", nullptr, enableWireframe)) {
               enableWireframe = !enableWireframe;
            }
            ImGui::EndMenu();
         }
         ImGui::EndMainMenuBar();
      }

      dialog->checkShouldOpen();

      if (showPopup) {
         ImGui::OpenPopup("Import Skeleton");
      }

      if (showAnimation) {
         ImGui::OpenPopup("Import Animation");
      }

      if (b) {
         ImGui::OpenPopup("Unsaved");
      }

      if (ImGui::BeginPopupModal("Unsaved", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
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
      if (show) {
         ImGui::ShowDemoWindow(&show);
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

   auto Manager::getSavePath() -> std::optional<std::filesystem::path> {
      auto outPath = NFD::UniquePath{};

      const auto result = SaveDialog(outPath, ProjectFileFilters.data(), ProjectFileFilters.size());
      if (result == NFD_OKAY) {
         Log.debug("Success: {0}", outPath.get());
         return std::optional{std::filesystem::path{outPath.get()}};
      }
      if (result == NFD_CANCEL) {
         Log.debug("User pressed Cancel");
         return std::nullopt;
      }
      Log.error("Error getting save path: {0}", NFD::GetError());
      return std::nullopt;
   }
}

// NOLINTEND(cppcoreguidelines-pro-type-vararg,
// hicpp-vararg,readability-function-cognitive-complexity,hicpp-no-array-decay,hicpp-no-pointer-decay,cppcoreguidelines-pro-bounds-array-to-pointer-decay,hicpp-signed-bitwise)