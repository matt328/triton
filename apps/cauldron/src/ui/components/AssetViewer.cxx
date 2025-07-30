#include "AssetViewer.hpp"

#include "PrefKeys.hpp"
#include "ui/assets/IconsLucide.hpp"
#include "ui/components/FileDialog.hpp"
#include "ui/components/dialog/Helpers.hpp"

namespace ed {

constexpr auto ItemIndent = 16.f;

AssetViewer::AssetViewer(std::shared_ptr<bk::Preferences> newPreferences,
                         std::shared_ptr<tr::IEventQueue> newEventQueue)
    : preferences{std::move(newPreferences)}, eventQueue{std::move(newEventQueue)} {
  Log.trace("Constructing AssetViewer");
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
          skeletonAliasInfo.shouldShow = true;
        }
        if (ImGui::MenuItem("Animation...")) {
          animationAliasInfo.shouldShow = true;
        }
        if (ImGui::MenuItem("Model...")) {
          modelAliasInfo.shouldShow = true;
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
        for (const auto& name :
             editorState.contextData.assets.skeletons | std::ranges::views::keys) {
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
        for (const auto& name :
             editorState.contextData.assets.animations | std::ranges::views::keys) {
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
        for (const auto& name : editorState.contextData.assets.models | std::ranges::views::keys) {
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

  renderModelDialog();
  renderSkeletonDialog();
  renderAnimationDialog();
}

auto AssetViewer::renderModelDialog() -> void {
  if (modelAliasInfo.shouldShow) {
    ImGui::OpenPopup("Model");
    modelAliasInfo.shouldShow = false;
    modelAliasInfo.isOpen = true;
  }

  bool shouldOk{};
  bool shouldCancel{};

  if (ImGui::BeginPopupModal("Model", &modelAliasInfo.isOpen, ImGuiConstants::ModalFlags)) {
    if (ImGui::IsKeyPressed(ImGuiKey_Escape)) {
      shouldCancel = true;
    }

    if (ImGui::IsWindowAppearing()) {
      ImGui::SetKeyboardFocusHere();
    }

    auto filterItems = {
        FilterItem{.filter = ".trm", .displayName = "Triton Models"},
        FilterItem{.filter = ".*", .displayName = "All Files"},
    };

    ImGui::InputText("Name", &modelAliasInfo.alias.alias);
    renderFileControl(prefs::assetViewerModelPath,
                      preferences,
                      filterItems,
                      modelAliasInfo.alias.filePath);

    ImGui::Separator();

    std::tie(shouldOk, shouldCancel) = renderOkCancelButtons();

    if (shouldOk) {
      Log.trace("shouldOk alias={}, path={}",
                modelAliasInfo.alias.alias,
                modelAliasInfo.alias.filePath.string());
      ImGui::CloseCurrentPopup();
      modelAliasInfo.isOpen = false;
      Log.trace("name={}, file={}",
                modelAliasInfo.alias.alias,
                modelAliasInfo.alias.filePath.string());
      eventQueue->emit(tr::AddModel{.name = modelAliasInfo.alias.alias,
                                    .fileName = modelAliasInfo.alias.filePath.string()});
      modelAliasInfo.alias = {};
    }

    if (shouldCancel) {
      Log.trace("shouldCancel");
      ImGui::CloseCurrentPopup();
      modelAliasInfo.isOpen = false;
      modelAliasInfo.alias = {};
    }

    ImGui::EndPopup();
  }
}

auto AssetViewer::renderSkeletonDialog() -> void {
  if (skeletonAliasInfo.shouldShow) {
    ImGui::OpenPopup("Skeleton");
    skeletonAliasInfo.shouldShow = false;
    skeletonAliasInfo.isOpen = true;
  }

  bool shouldOk{};
  bool shouldCancel{};

  if (ImGui::BeginPopupModal("Skeleton", &skeletonAliasInfo.isOpen, ImGuiConstants::ModalFlags)) {
    if (ImGui::IsKeyPressed(ImGuiKey_Escape)) {
      shouldCancel = true;
    }

    if (ImGui::IsWindowAppearing()) {
      ImGui::SetKeyboardFocusHere();
    }

    auto filterItems = {
        FilterItem{.filter = ".ozz", .displayName = "Ozz Skeletons"},
        FilterItem{.filter = ".*", .displayName = "All Files"},
    };

    ImGui::InputText("Name", &skeletonAliasInfo.alias.alias);
    renderFileControl(prefs::assetViewerSkeletonPath,
                      preferences,
                      filterItems,
                      skeletonAliasInfo.alias.filePath);

    ImGui::Separator();

    std::tie(shouldOk, shouldCancel) = renderOkCancelButtons();

    if (shouldOk) {
      Log.trace("shouldOk alias={}, path={}",
                skeletonAliasInfo.alias.alias,
                skeletonAliasInfo.alias.filePath.string());
      ImGui::CloseCurrentPopup();
      skeletonAliasInfo.isOpen = false;
      Log.trace("name={}, file={}",
                skeletonAliasInfo.alias.alias,
                skeletonAliasInfo.alias.filePath.string());
      eventQueue->emit(tr::AddSkeleton{.name = skeletonAliasInfo.alias.alias,
                                       .fileName = skeletonAliasInfo.alias.filePath.string()});
    }

    if (shouldCancel) {
      Log.trace("shouldCancel");
      ImGui::CloseCurrentPopup();
      skeletonAliasInfo.isOpen = false;
    }

    ImGui::EndPopup();
  }
}

auto AssetViewer::renderAnimationDialog() -> void {
  if (animationAliasInfo.shouldShow) {
    ImGui::OpenPopup("Animation");
    animationAliasInfo.shouldShow = false;
    animationAliasInfo.isOpen = true;
  }

  bool shouldOk{};
  bool shouldCancel{};

  if (ImGui::BeginPopupModal("Animation", &animationAliasInfo.isOpen, ImGuiConstants::ModalFlags)) {
    if (ImGui::IsKeyPressed(ImGuiKey_Escape)) {
      shouldCancel = true;
    }

    if (ImGui::IsWindowAppearing()) {
      ImGui::SetKeyboardFocusHere();
    }

    auto filterItems = {
        FilterItem{.filter = ".ozz", .displayName = "Ozz Animations"},
        FilterItem{.filter = ".*", .displayName = "All Files"},
    };

    ImGui::InputText("Name", &animationAliasInfo.alias.alias);
    renderFileControl(prefs::assetViewerAnimationPath,
                      preferences,
                      filterItems,
                      animationAliasInfo.alias.filePath);

    ImGui::Separator();

    std::tie(shouldOk, shouldCancel) = renderOkCancelButtons();

    if (shouldOk) {
      Log.trace("shouldOk alias={}, path={}",
                animationAliasInfo.alias.alias,
                animationAliasInfo.alias.filePath.string());
      ImGui::CloseCurrentPopup();
      animationAliasInfo.isOpen = false;
      Log.trace("name={}, file={}",
                animationAliasInfo.alias.alias,
                animationAliasInfo.alias.filePath.string());
      eventQueue->emit(tr::AddAnimation{.name = animationAliasInfo.alias.alias,
                                        .fileName = animationAliasInfo.alias.filePath.string()});
    }

    if (shouldCancel) {
      Log.trace("shouldCancel");
      ImGui::CloseCurrentPopup();
      animationAliasInfo.isOpen = false;
    }

    ImGui::EndPopup();
  }
}

}
