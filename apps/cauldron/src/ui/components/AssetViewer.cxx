#include "AssetViewer.hpp"

#include "DialogManager.hpp"

#include "ui/assets/IconsLucide.hpp"
#include "ui/components/FileDialog.hpp"

namespace ed {

constexpr auto ItemIndent = 16.f;

AssetViewer::AssetViewer(std::shared_ptr<DialogManager> newDialogManager,
                         std::shared_ptr<Properties> newProperties,
                         std::shared_ptr<tr::IEventQueue> newEventQueue)
    : dialogManager{std::move(newDialogManager)},
      properties{std::move(newProperties)},
      eventQueue{std::move(newEventQueue)} {
  Log.trace("Constructing AssetViewer");
  createSkeletonDialog();
  createAnimationDialog();
  createModelDialog();
}

AssetViewer::~AssetViewer() {
  Log.trace("Destroying AssetViewer");
}

auto AssetViewer::bindInput() -> void {
}

auto AssetViewer::render(const tr::EditorState& editorState) -> void {
  if (const auto unsaved = !editorState.contextData.saved ? ImGuiWindowFlags_UnsavedDocument : 0;
      ImGui::Begin("Assets", nullptr, ImGuiWindowFlags_MenuBar | unsaved)) {

    if (ImGui::BeginMenuBar()) {
      if (ImGui::BeginMenu("Import")) {
        if (ImGui::MenuItem("Skeleton...")) {
          dialogManager->setOpen("Skeleton");
        }
        if (ImGui::MenuItem("Animation...")) {
          dialogManager->setOpen("Animation");
        }
        if (ImGui::MenuItem("Model...")) {
          dialogManager->setOpen("Model");
        }
        ImGui::EndMenu();
      }
      ImGui::EndMenuBar();
    }

    static auto headerState = std::array{true, true, true, true};

    {
      ImGui::SetNextItemOpen(headerState[0]);
      if (ImGui::CollapsingHeader("Scripts")) {
        ImGui::Indent(ItemIndent);
        ImGui::Selectable(ICON_LC_FILE_CODE " MainCharacter");
        ImGui::Selectable(ICON_LC_FILE_CODE " MillAbout");
        ImGui::Selectable(ICON_LC_FILE_CODE " IdleInPlace");
        ImGui::Unindent(ItemIndent);
        headerState[0] = true;
      } else {
        headerState[0] = false;
      }
    }

    {
      ImGui::SetNextItemOpen(headerState[1]);
      if (ImGui::CollapsingHeader("Skeletons")) {
        ImGui::Indent(ItemIndent);
        for (const auto& name : editorState.contextData.assets.skeletons | std::views::keys) {
          ImGui::Selectable((std::string{ICON_LC_BONE} + " " + name).c_str());
        }
        ImGui::Unindent(ItemIndent);
        headerState[1] = true;
      } else {
        headerState[1] = false;
      }
    }

    {
      ImGui::SetNextItemOpen(headerState[2]);
      if (ImGui::CollapsingHeader("Animations")) {
        ImGui::Indent(ItemIndent);
        for (const auto& name : editorState.contextData.assets.animations | std::views::keys) {
          ImGui::Selectable((std::string{ICON_LC_FILE_VIDEO} + " " + name).c_str());
        }
        ImGui::Unindent(ItemIndent);
        headerState[2] = true;
      } else {
        headerState[2] = false;
      }
    }

    {
      ImGui::SetNextItemOpen(headerState[3]);
      if (ImGui::CollapsingHeader("Models")) {
        ImGui::Indent(ItemIndent);
        for (const auto& name : editorState.contextData.assets.models | std::views::keys) {
          ImGui::Selectable((std::string{ICON_LC_BOX} + " " + name).c_str());
        }
        ImGui::Unindent(ItemIndent);
        headerState[3] = true;
      } else {
        headerState[3] = false;
      }
    }
  }
  ImGui::End();
}

void AssetViewer::createSkeletonDialog() {
  auto dialog = std::make_unique<ModalDialog>(
      ICON_LC_BONE,
      " Skeleton",
      [&](const ModalDialog& dialog) {
        Log.trace("name: {0}, file: {1}",
                  dialog.getValue<std::string>("name").value(),
                  dialog.getValue<std::filesystem::path>("filename").value().string());
        eventQueue->emit(tr::AddSkeleton{
            .name = dialog.getValue<std::string>("name").value(),
            .fileName = dialog.getValue<std::filesystem::path>("filename").value().string()});
      },
      []() { Log.debug("Cancelled Dialog with no input"); });

  auto filterItems = {
      FilterItem{.filter = ".ozz", .displayName = "Ozz Skeletons"},
      FilterItem{.filter = ".*", .displayName = "All Files"},
  };

  dialog->addControl("name", "Skeleton Name", std::string("Unnamed Skeleton"));
  dialog->addFileControl("filename", "Skeleton File", properties, filterItems);

  dialogManager->addDialog("Skeleton", std::move(dialog));
}

void AssetViewer::createAnimationDialog() {
  auto dialog = std::make_unique<ModalDialog>(
      ICON_LC_FILE_VIDEO,
      " Animation",
      [&](const ModalDialog& dialog) {
        Log.trace("name: {0}, file: {1}",
                  dialog.getValue<std::string>("name").value(),
                  dialog.getValue<std::filesystem::path>("filename").value().string());
        eventQueue->emit(tr::AddAnimation{
            .name = dialog.getValue<std::string>("name").value(),
            .fileName = dialog.getValue<std::filesystem::path>("filename").value().string(),
        });
      },
      []() { Log.debug("Cancelled Dialog with no input"); });

  auto filterItems = {
      FilterItem{.filter = ".ozz", .displayName = "Ozz Animation"},
      FilterItem{.filter = ".*", .displayName = "All Files"},
  };

  dialog->addControl("name", "Animation Name", std::string("Unnamed Animation"));
  dialog->addFileControl("filename", "Animation File", properties, filterItems);

  dialogManager->addDialog("Animation", std::move(dialog));
}

void AssetViewer::createModelDialog() {
  auto dialog = std::make_unique<ModalDialog>(
      ICON_LC_FILE_BOX,
      " Model",
      [&](const ModalDialog& dialog) {
        Log.trace("name: {0}, file: {1}",
                  dialog.getValue<std::string>("name").value(),
                  dialog.getValue<std::filesystem::path>("filename").value().string());
        eventQueue->emit(tr::AddModel{
            .name = dialog.getValue<std::string>("name").value(),
            .fileName = dialog.getValue<std::filesystem::path>("filename").value().string()});
      },
      []() { Log.debug("Cancelled Dialog with no input"); });

  auto filterItems = {
      FilterItem{.filter = ".trm", .displayName = "Triton Models"},
      FilterItem{.filter = ".*", .displayName = "All Files"},
  };

  dialog->addControl("name", "Model Name", std::string("Unnamed Model"));
  dialog->addFileControl("filename", "Model File", properties, filterItems);

  dialogManager->addDialog("Model", std::move(dialog));
}
}
