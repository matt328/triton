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
      eventQueue->emit(tr::LoadProject{.filePath = selectedFile.front()});
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
      eventQueue->emit(tr::SaveProject{.filePath = filepath});
      properties->setRecentFile(filepath);
    } catch (const std::exception& ex) { Log.error(ex.what()); }
  });
}

Menu::~Menu() {
  Log.trace("Destroying Menu");
}

auto Menu::bindInput() -> void {
}

auto Menu::renderFileMenu(const tr::EditorState& editorState, bool& showConfirmDialog) -> void {
  if (ImGui::BeginMenu("File")) {

    if (ImGui::MenuItem("New Project...")) {
      if (!editorState.contextData.saved) {
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
          eventQueue->emit(tr::LoadProject{.filePath = recentFile.value()});
          openFilePath.emplace(recentFile.value());
        }
      }
      ImGui::EndMenu();
    }

    ImGui::Separator();
    if (ImGui::MenuItem("Save Project", "Ctrl+S", false, !editorState.contextData.saved)) {
      if (openFilePath.has_value()) {
        eventQueue->emit(tr::SaveProject{.filePath = openFilePath.value()});
      } else {
        projectSaveDialog->setOpen(std::nullopt, std::string{ICON_LC_FILE} + " Save Project");
      }
    }

    if (ImGui::MenuItem("Save Project As...", nullptr, false, !editorState.contextData.saved)) {
      projectSaveDialog->setOpen(std::nullopt, std::string{ICON_LC_FILE} + " Save Project");
    }

    ImGui::Separator();

    if (ImGui::MenuItem("Exit", "Alt+F4")) {
      eventQueue->emit(tr::WindowClosed{});
    }
    ImGui::EndMenu();
  }
}

auto Menu::renderViewMenu() -> void {
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
}

auto Menu::renderUnsavedChangesPopup() -> void {
  if (ImGui::BeginPopupModal("Unsaved", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
    ImGui::Text("Unsaved changes will be lost. Are you sure?");
    ImGui::Separator();

    auto availableWidth = ImGui::GetContentRegionAvail().x;

    ImGui::SetCursorPosX(availableWidth - (ImGuiConstants::ButtonWidth * 2));

    if (ImGui::Button(ICON_LC_CIRCLE_CHECK_BIG " OK", ImVec2(ImGuiConstants::ButtonWidth, 0.f))) {
      ImGui::CloseCurrentPopup();
      Log.info("Ok");
    }
    ImGui::SetItemDefaultFocus();
    ImGui::SameLine();
    if (ImGui::Button(ICON_LC_BAN " Cancel", ImVec2(ImGuiConstants::ButtonWidth, 0.f))) {
      ImGui::CloseCurrentPopup();
    }
    ImGui::EndPopup();
  }
}

auto Menu::renderDialogs() -> void {
  projectOpenDialog->checkShouldOpen();
  projectOpenDialog->render();

  projectSaveDialog->checkShouldOpen();
  projectSaveDialog->render();
}

void Menu::render(const tr::EditorState& uiState) {
  auto showConfirmDialog = false;
  if (ImGui::BeginMainMenuBar()) {
    renderFileMenu(uiState, showConfirmDialog);
    renderViewMenu();
    ImGui::EndMainMenuBar();
  }

  renderUnsavedChangesPopup();
  renderDialogs();

  if (showConfirmDialog) {
    ImGui::OpenPopup("Unsaved");
  }

  if (state.demoWindowVisible) {
    ImGui::ShowDemoWindow(&state.demoWindowVisible);
  }
}
}
