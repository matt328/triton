#include "Menu.hpp"

#include "api/fx/IEventQueue.hpp"
#include "Properties.hpp"
#include "FileDialog.hpp"

namespace ed {

const auto ProjectFilters =
    std::vector{FilterItem{.filter = ".trp", .displayName = "Triton Projects"},
                FilterItem{.filter = ".*", .displayName = "All Files"}};

Menu::Menu(std::shared_ptr<Properties> newProperties,
           std::shared_ptr<tr::IEventQueue> newEventQueue)
    : properties{std::move(newProperties)}, eventQueue{std::move(newEventQueue)} {

  projectOpenDialog = std::make_unique<FileDialog>(properties, ProjectFilters, "project");
  projectOpenDialog->setOnOk([&](std::vector<std::filesystem::path> selectedFile) {
    try {
      Log.trace("Open project file: {}", selectedFile.front().string());
      openFilePath = selectedFile.front();
      // eventQueue->emit(LoadProject{.fileName = selectedFile.front()});
    } catch (const std::exception& ex) { Log.error(ex.what()); }
  });

  projectSaveDialog = std::make_unique<FileDialog>(properties, ProjectFilters, "project");
  projectSaveDialog->setOnOk([&](std::vector<std::filesystem::path> selectedFile) {
    try {
      auto filepath = selectedFile.front();
      if (!filepath.has_extension()) {
        filepath.replace_extension("trp");
      }
      Log.trace("Save project file: {}", filepath.string());
      // eventQueue->emit(SaveProject{.filePath = filePath});
      properties->setRecentFile(filepath);
    } catch (const std::exception& ex) { Log.error(ex.what()); }
  });
}

Menu::~Menu() {
  Log.trace("Destroying Menu");
}

auto Menu::bindInput() -> void {
}

void Menu::render(const tr::EditorState& uiState) {
  auto showConfirmDialog = false;

  if (ImGui::BeginMainMenuBar()) {
    if (ImGui::BeginMenu("File")) {

      if (ImGui::MenuItem("New Project...")) {
        if (!uiState.contextData.saved) {
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
            // eventQueue->emit(LoadProject{.fileName = recentFile.value()});
            openFilePath.emplace(recentFile.value());
          }
        }
        ImGui::EndMenu();
      }

      ImGui::Separator();
      if (ImGui::MenuItem("Save Project", "Ctrl+S", false, !uiState.contextData.saved)) {
        if (openFilePath.has_value()) {
          // eventQueue->emit(SaveProject{.fileName = openFilePath.value()});
        } else {
          projectSaveDialog->setOpen(std::nullopt, std::string{ICON_LC_FILE} + " Save Project");
        }
      }

      if (ImGui::MenuItem("Save Project As...", nullptr, false, !uiState.contextData.saved)) {
        projectSaveDialog->setOpen(std::nullopt, std::string{ICON_LC_FILE} + " Save Project");
      }

      ImGui::Separator();

      if (ImGui::MenuItem("Exit", "Alt+F4")) {
        eventQueue->emit(tr::WindowClosed{});
      }
      ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("View")) {
      if (ImGui::MenuItem("Fullscreen", "Alt+Enter", this->fullscreen)) {
        this->fullscreen = !this->fullscreen;
        toggleFullscreenFn();
      }
      if (ImGui::MenuItem("Demo Window", nullptr, state.demoWindowVisible)) {
        state.demoWindowVisible = !state.demoWindowVisible;
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

  if (state.demoWindowVisible) {
    ImGui::ShowDemoWindow(&state.demoWindowVisible);
  }
}
}
