#include "Menu.hpp"
#include "data/DataFacade.hpp"

namespace ed {

const auto ProjectFilters =
    std::vector{FilterItem{.filter = ".trp", .displayName = "Triton Projects"},
                FilterItem{.filter = ".*", .displayName = "All Files"}};

Menu::Menu(std::shared_ptr<DataFacade> newDataFacade,
           std::shared_ptr<Properties> newProperties,
           std::shared_ptr<DialogManager> newDialogManager,
           std::shared_ptr<tr::IEventBus> newEventBus)
    : dataFacade{std::move(newDataFacade)},
      properties{std::move(newProperties)},
      dialogManager{std::move(newDialogManager)},
      eventBus{std::move(newEventBus)} {

  projectOpenDialog = std::make_unique<FileDialog>(properties, ProjectFilters, "project");
  projectOpenDialog->setOnOk([&](std::vector<std::filesystem::path> selectedFile) {
    try {
      Log.trace("Open project file: {}", selectedFile.front().string());
      dataFacade->load(selectedFile.front());
    } catch (const std::exception& ex) { Log.error(ex.what()); }
  });

  projectSaveDialog = std::make_unique<FileDialog>(properties, ProjectFilters, "project");
  projectSaveDialog->setOnOk([&](std::vector<std::filesystem::path> selectedFile) {
    try {
      Log.trace("Save project file: {}", selectedFile.front().string());
      dataFacade->save(selectedFile.front());
    } catch (const std::exception& ex) { Log.error(ex.what()); }
  });
}

Menu::~Menu() {
  Log.trace("Destroying Menu");
}

void Menu::render() {
  auto showConfirmDialog = false;
  static auto showDemoWindow = false;

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

      if (ImGui::MenuItem("New Project...")) {
        if (dataFacade->isUnsaved()) {
          showConfirmDialog = true;
        }
      }

      ImGui::Separator();
      if (ImGui::MenuItem("Open Project...")) {
        projectOpenDialog->setOpen(std::nullopt, "Open Project File");
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
          projectSaveDialog->setOpen(std::nullopt, std::string{ICON_LC_FILE} + " Save Project");
        }
      }

      if (ImGui::MenuItem("Save Project As...", nullptr, false, dataFacade->isUnsaved())) {
        projectSaveDialog->setOpen(std::nullopt, std::string{ICON_LC_FILE} + " Save Project");
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
      if (ImGui::MenuItem("Demo Window", nullptr, showDemoWindow)) {
        showDemoWindow = !showDemoWindow;
      }
      if (ImGui::MenuItem("Wireframe", nullptr, enableWireframe)) {
        enableWireframe = !enableWireframe;
      }
      ImGui::EndMenu();
    }
    ImGui::EndMainMenuBar();
  }

  if (showConfirmDialog) {
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

  projectOpenDialog->checkShouldOpen();
  projectOpenDialog->render();

  projectSaveDialog->checkShouldOpen();
  projectSaveDialog->render();

  if (showDemoWindow) {
    ImGui::ShowDemoWindow(&showDemoWindow);
  }
}
}
