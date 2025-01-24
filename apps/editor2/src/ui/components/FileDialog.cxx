#include <filesystem>
#include <optional>
#include <platform_folders.h>
#include <utility>

#include "FileDialog.hpp"
#include "imgui.h"
#include "ui/assets/IconsLucide.hpp"

namespace ed {

FileDialog::FileDialog(std::shared_ptr<Properties> newProperties,
                       std::vector<FilterItem> newFilterItems,
                       std::string_view newUniqueName)
    : properties{std::move(newProperties)},
      filterItems{std::move(newFilterItems)},
      uniqueName{newUniqueName.data()} {
  lastPathKey = uniqueName + "lastPathKey";
}

auto FileDialog::render() -> void {
  if (!isOpen) {
    return;
  }

  bool shouldOk{};
  bool shouldCancel{};

  if (ImGui::BeginPopupModal(label.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
    // File Path
    // Try initial path passed in.
    if (initialPath.has_value()) {
      if (!currentFolder.has_value()) {
        currentFolder = initialPath;
      }
      // Try properties
    } else if (properties->get(lastPathKey).has_value()) {
      if (!currentFolder.has_value()) {
        currentFolder = properties->get(lastPathKey).transform([](const auto& path) {
          return std::filesystem::path{path};
        });
      }
      // Finally use desktop
    } else if (!currentFolder.has_value()) {
      currentFolder = sago::getDesktopFolder();
    }

    if (currentFolder.has_value()) {
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
        selectedFilename = "";
      }
    }
    for (size_t i = 0; i < folders.size(); ++i) {
      const auto label = std::string(ICON_LC_FOLDER) + " " + folders[i].path().stem().string();
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
          selectedFilename = "";
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

      const auto filterString = filterItems[selectedFilterIndex].filter;
      std::vector<std::filesystem::directory_entry> filteredFiles;
      std::copy_if(files.begin(),
                   files.end(),
                   std::back_inserter(filteredFiles),
                   [&filterString](const std::filesystem::directory_entry& entry) {
                     if (filterString == ".*") {
                       return true;
                     }
                     return entry.path().extension() == filterString;
                   });

      size_t i = 0;
      for (const auto& entry : filteredFiles) {
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
          currentFile = filteredFiles[i].path();
          selectedFilename = currentFile->filename().string();
        }

        ImGui::TableNextColumn();
        ImGui::TextUnformatted(entry.path().extension().string().c_str());

        ImGui::TableNextColumn();
        ImGui::TextUnformatted(std::to_string(entry.file_size()).c_str());

        ++i;
      }
      ImGui::EndTable();
    }

    ImGui::AlignTextToFramePadding();
    ImGui::Text("Filename");
    ImGui::SameLine();

    {
      auto availableWidth = ImGui::GetContentRegionAvail().x;
      auto dropdownWidth = 200.f;

      ImGui::PushItemWidth(availableWidth - dropdownWidth - ImGui::GetStyle().ItemSpacing.x);
      ImGui::InputText("##filename", &selectedFilename);

      ImGui::SameLine();

      ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x -
                           dropdownWidth);

      auto items = std::vector<std::string>{};
      for (const auto& filterItem : filterItems) {
        items.push_back(filterItem.getString());
      }

      auto comboFlags = 0;
      const char* initialValue = items[selectedFilterIndex].c_str();

      ImGui::SetNextItemWidth(dropdownWidth);
      if (ImGui::BeginCombo("##filetype", initialValue, comboFlags)) {
        for (size_t n = 0; n < items.size(); n++) {
          const bool isSelected = (selectedFilterIndex == n);
          if (ImGui::Selectable(items[n].c_str(), isSelected)) {
            selectedFilterIndex = n;
          }
          if (isSelected) {
            ImGui::SetItemDefaultFocus();
          }
        }
        ImGui::EndCombo();
        ImGui::PopItemWidth();
      }
    }

    ImGui::Separator();

    auto availableWidth = ImGui::GetContentRegionAvail().x;
    auto buttonWidth = 80.f;

    ImGui::SetCursorPosX(availableWidth - buttonWidth * 2);

    if (ImGui::Button(ICON_LC_CIRCLE_CHECK_BIG " OK", ImVec2(buttonWidth, 0.f))) {
      shouldOk = true;
    }

    ImGui::SameLine();
    if (ImGui::Button(ICON_LC_BAN " Cancel", ImVec2(buttonWidth, 0.f))) {
      shouldCancel = true;
    }

    ImGui::EndPopup();
  }

  if (shouldOk) {
    // TODO(matt) Handle save case here. currently in save case, selectedFileName will be
    // set here not currentFile. See if open case can just use selectedFileName as well
    ImGui::CloseCurrentPopup();
    const auto propPath = currentFolder.value() / selectedFilename;
    Log.trace("putting property {}, value: {}", lastPathKey, currentFolder.value().string());
    properties->put(lastPathKey, currentFolder.value().string());
    isOpen = false;
    if (onOk.has_value()) {
      onOk.value()({propPath});
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

auto FileDialog::setOpen(std::optional<std::filesystem::path> newInitialPath,
                         std::optional<std::string> newLabel) -> void {
  isOpen = true;
  initialPath = std::move(newInitialPath);
  if (newLabel.has_value()) {
    label = newLabel.value();
  }
}

auto FileDialog::checkShouldOpen() -> void {
  if (isOpen) {
    ImGui::OpenPopup(label.c_str());
  }
}
}
