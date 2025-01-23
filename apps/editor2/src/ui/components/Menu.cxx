#include "Menu.hpp"
#include "data/DataFacade.hpp"

namespace ed {

Menu::Menu(std::shared_ptr<DataFacade> newDataFacade,
           std::shared_ptr<Properties> newProperties,
           std::shared_ptr<DialogManager> newDialogManager,
           std::shared_ptr<tr::IEventBus> newEventBus)
    : dataFacade{std::move(newDataFacade)},
      properties{std::move(newProperties)},
      dialogManager{std::move(newDialogManager)},
      eventBus{std::move(newEventBus)} {
  fileDialog = std::make_unique<FileDialog>(properties);
}

Menu::~Menu() {
  Log.trace("Destroying Menu");
}

void Menu::render() {
  auto b = false;
  auto showPopup = false;
  auto showAnimation = false;
  static auto show = false;

  static auto fsPath = std::filesystem::path{"C:/Users/matt/Projects/game-assets/models"};

  if (ImGui::BeginMainMenuBar()) {
    if (ImGui::BeginMenu("File")) {

      if (ImGui::MenuItem("Test Dialog")) {
        dialogManager->setOpen("Test Dialog");
      }

      if (ImGui::MenuItem("Create Terrain")) {
        dataFacade->createTerrain("Test Terrain");
      }

      if (ImGui::MenuItem("Create Box")) {
        dataFacade->createAABB();
      }

      if (ImGui::MenuItem("Import Skeleton")) {
        showPopup = true;
      }

      if (ImGui::MenuItem("Import Animation")) {
        showAnimation = true;
      }

      if (ImGui::MenuItem("New Project...")) {
        if (dataFacade->isUnsaved()) {
          b = true;
        }
      }

      ImGui::Separator();
      if (ImGui::MenuItem("Open Project...")) {
        // auto inPath = NFD::UniquePath{};
        // constexpr auto filterSize = static_cast<nfdfiltersize_t>(ProjectFileFilters.size());
        // if (const auto result = OpenDialog(inPath, ProjectFileFilters.data(), filterSize);
        //     result == NFD_OKAY) {
        //   const auto filePath = std::filesystem::path{inPath.get()};
        //   dataFacade->load(filePath);
        //   openFilePath = filePath;
        //   properties->setRecentFile(filePath);
        // } else {
        //   Log.error("File Dialog Error: ", NFD::GetError());
        // }
      }

      if (ImGui::BeginMenu("Open Recent")) {
        if (const auto recentFile = properties->getRecentFile(); recentFile.has_value()) {
          const auto nameOnly = recentFile.value().string();
          if (ImGui::MenuItem(nameOnly.c_str())) {
            dataFacade->clear();
            dataFacade->load(recentFile.value());
            openFilePath.emplace(recentFile.value());
          }
        }
        ImGui::EndMenu();
      }

      ImGui::Separator();
      if (ImGui::MenuItem("Save Project", "Ctrl+S", false, dataFacade->isUnsaved())) {
        if (openFilePath.has_value()) {
          dataFacade->save(openFilePath.value());
        } else {
          fileDialog->setOnOk([&](std::vector<std::filesystem::path> selectedFile) {
            try {
              dataFacade->save(selectedFile.front());
            } catch (const std::exception& ex) { Log.error(ex.what()); }
          });
          fileDialog->setOpen(std::nullopt, std::string{ICON_LC_FILE} + " Save Project");
        }
      }

      if (ImGui::MenuItem("Save Project As...", nullptr, false, dataFacade->isUnsaved())) {
        fileDialog->setOnOk([&](std::vector<std::filesystem::path> selectedFile) {
          try {
            dataFacade->save(selectedFile.front());
          } catch (const std::exception& ex) { Log.error(ex.what()); }
        });
        fileDialog->setOpen(std::nullopt, std::string{ICON_LC_FILE} + " Save Project");
      }

      ImGui::Separator();

      if (ImGui::MenuItem("Exit", "Alt+F4")) {
        eventBus->emit(tr::WindowClosed{});
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

    auto availableWidth = ImGui::GetContentRegionAvail().x;
    auto buttonWidth = 80.f;

    ImGui::SetCursorPosX(availableWidth - buttonWidth * 2);

    if (ImGui::Button(ICON_LC_CIRCLE_CHECK_BIG " OK", ImVec2(buttonWidth, 0.f))) {
      ImGui::CloseCurrentPopup();
      Log.info("Ok");
    }
    ImGui::SetItemDefaultFocus();
    ImGui::SameLine();
    if (ImGui::Button(ICON_LC_BAN " Cancel", ImVec2(buttonWidth, 0.f))) {
      ImGui::CloseCurrentPopup();
    }
    ImGui::EndPopup();
  }

  fileDialog->checkShouldOpen();
  fileDialog->render();

  if (show) {
    ImGui::ShowDemoWindow(&show);
  }
}

}
