#include <optional>
#include <utility>

#include "FileDialog.hpp"
#include "imgui.h"
#include "ui/assets/IconsLucide.hpp"

namespace ed {

auto FileDialog::render() -> void {
  if (!isOpen) {
    return;
  }

  bool shouldOk{};
  bool shouldCancel{};

  if (ImGui::BeginPopupModal(label.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
    // File Path
    if (initialPath.has_value()) {
      if (!currentFolder.has_value()) {
        currentFolder = initialPath;
      }

      ImGui::Text("%s", currentFolder.value().string().c_str());
    }

    // Folder List
    // Get Files and Folders
    std::vector<std::filesystem::directory_entry> files;
    std::vector<std::filesystem::directory_entry> folders;

    try {
      for (const auto& p : std::filesystem::directory_iterator(currentFolder.value())) {
        if (p.is_directory()) {
          folders.push_back(p);
        } else {
          files.push_back(p);
        }
      }
    } catch (const std::exception& ex) { Log.warn("Error listing directories: {}", ex.what()); }

    // Render folder list
    ImGui::BeginChild("Directories##1", ImVec2(200, 300), 1, ImGuiWindowFlags_HorizontalScrollbar);

    if (ImGui::Selectable("..",
                          false,
                          ImGuiSelectableFlags_AllowDoubleClick,
                          ImVec2(ImGui::GetContentRegionAvail().x, 0))) {
      if (ImGui::IsMouseDoubleClicked(0)) {
        currentFolder = currentFolder.value().parent_path();
      }
    }
    for (size_t i = 0; i < folders.size(); ++i) {
      const auto label =
          std::string(ICON_LC_FOLDER) + " " + folders[i].path().stem().string().c_str();
      if (ImGui::Selectable(label.c_str(),
                            i == folderSelectedIndex,
                            ImGuiSelectableFlags_AllowDoubleClick,
                            ImVec2(ImGui::GetContentRegionAvail().x, 0))) {
        if (ImGui::IsMouseDoubleClicked(0)) {
          currentFile = std::nullopt;
          currentFolder = folders[i].path();
          folderSelectedIndex = 0;
          fileSelectedIndex = 0;
          ImGui::SetScrollHereY(0.0f);
        } else {
          // Single Clicked
          folderSelectedIndex = i;
        }
      }
    }
    ImGui::EndChild();

    ImGui::SameLine();

    auto flags = ImGuiTableFlags_BordersOuter | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollX |
                 ImGuiTableFlags_ScrollY | ImGuiTableFlags_NoHostExtendX |
                 ImGuiTableFlags_NoHostExtendY;

    if (ImGui::BeginTable("Table", 3, flags, ImVec2(516, 300))) {

      const auto colFlags = ImGuiTableColumnFlags_WidthStretch;
      ImGui::TableSetupScrollFreeze(0, 1); // Make top row always visible
      ImGui::TableSetupColumn("File", colFlags, 280.f);
      ImGui::TableSetupColumn("Type", colFlags, 80.f);
      ImGui::TableSetupColumn("Size", colFlags, 100.f);
      ImGui::TableHeadersRow();

      size_t i = 0;

      for (const auto& entry : files) {
        ImGui::TableNextRow();

        ImGui::TableNextColumn();

        const auto label =
            std::string(ICON_LC_FILE) + " " + entry.path().filename().string().c_str();

        if (ImGui::Selectable(label.c_str(),
                              i == fileSelectedIndex,
                              ImGuiSelectableFlags_SpanAllColumns |
                                  ImGuiSelectableFlags_AllowOverlap,
                              ImVec2(0, 18.f))) {
          fileSelectedIndex = i;
          currentFile = files[i].path();
        }

        ImGui::TableNextColumn();
        ImGui::TextUnformatted(entry.path().extension().string().c_str());

        ImGui::TableNextColumn();
        ImGui::TextUnformatted(std::to_string(entry.file_size()).c_str());

        ++i;
      }
      ImGui::EndTable();
    }

    if (!files.empty() && fileSelectedIndex < files.size()) {
      ImGui::Text("%s", files[fileSelectedIndex].path().string().c_str());
    }

    if (ImGui::Button(ICON_LC_CHECK " OK")) {
      shouldOk = true;
    }
    ImGui::SameLine();
    if (ImGui::Button(ICON_LC_X "Cancel")) {
      shouldCancel = true;
    }

    ImGui::EndPopup();
  }

  if (shouldOk) {
    ImGui::CloseCurrentPopup();
    isOpen = false;
    if (onOk.has_value()) {
      onOk.value()({currentFile.value()});
    }
  }

  if (shouldCancel) {
    ImGui::CloseCurrentPopup();
    isOpen = false;
    if (onCancel.has_value()) {
      onCancel.value()();
    }
  }
}

auto FileDialog::setOnOk(const FileDialogOkFunction& fn) -> void {
  onOk = fn;
}

auto FileDialog::setOpen(std::optional<std::filesystem::path> newInitialPath) -> void {
  isOpen = true;
  initialPath = std::move(newInitialPath);
}

auto FileDialog::checkShouldOpen() -> void {
  if (isOpen) {
    ImGui::OpenPopup(label.c_str());
  }
}
}
