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
}

Menu::~Menu() {
  Log.trace("Destroying Menu");
}

void Menu::render() {
  auto b = false;
  auto showPopup = false;
  auto showAnimation = false;
  static auto show = false;

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
        auto inPath = NFD::UniquePath{};
        constexpr auto filterSize = static_cast<nfdfiltersize_t>(ProjectFileFilters.size());
        if (const auto result = OpenDialog(inPath, ProjectFileFilters.data(), filterSize);
            result == NFD_OKAY) {
          const auto filePath = std::filesystem::path{inPath.get()};
          dataFacade->load(filePath);
          openFilePath = filePath;
          properties->setRecentFile(filePath);
        } else {
          Log.error("File Dialog Error: ", NFD::GetError());
        }
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
          const auto savePath = getSavePath();
          try {
            if (savePath.has_value()) {
              dataFacade->save(savePath.value());
            }
          } catch (const std::exception& ex) { Log.error(ex.what()); }
        }
      }

      if (ImGui::MenuItem("Save Project As...", nullptr, false, dataFacade->isUnsaved())) {
        const auto savePath = getSavePath();
        try {
          if (savePath.has_value()) {
            dataFacade->save(savePath.value());
          }
        } catch (const std::exception& ex) { Log.error(ex.what()); }
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

auto Menu::getSavePath() -> std::optional<std::filesystem::path> {
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
