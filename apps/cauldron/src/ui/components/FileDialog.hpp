#pragma once

#include "Preferences.hpp"
#include "ui/assets/IconsLucide.hpp"
namespace ed {

using FileDialogOkFunction = std::function<void(const std::vector<std::filesystem::path>&)>;
using FileDialogCancelFunction = std::function<void(void)>;

enum class SortOrder {
  None,
  Ascending,
  Descending
};

struct ColumnInfo {
  SortOrder file;
  SortOrder size;
  SortOrder date;
  SortOrder type;

  float fileWidth = 230.f;
  float sizeWidth = 80.f;
  float dateWidth = 80.f;
  float typeWidth;

  auto toggleFile() -> void {
    file = file == SortOrder::Ascending ? SortOrder::Descending : SortOrder::Ascending;
    size = SortOrder::None;
    date = SortOrder::None;
    type = SortOrder::None;
  }

  auto toggleSize() -> void {
    file = SortOrder::None;
    size = size == SortOrder::Ascending ? SortOrder::Descending : SortOrder::Ascending;
    date = SortOrder::None;
    type = SortOrder::None;
  }

  auto toggleDate() -> void {
    file = SortOrder::None;
    size = SortOrder::None;
    date = date == SortOrder::Ascending ? SortOrder::Descending : SortOrder::Ascending;
    type = SortOrder::None;
  }

  auto toggleType() -> void {
    file = SortOrder::None;
    size = SortOrder::None;
    date = SortOrder::None;
    type = type == SortOrder::Ascending ? SortOrder::Descending : SortOrder::Ascending;
  }
};

struct FolderItem {
  std::filesystem::directory_entry entry;
  bool selected{};
};

struct FileItem {
  std::filesystem::directory_entry entry;
  bool selected{};
};

struct FilterItem {
  std::string filter;
  std::string displayName;

  [[nodiscard]] auto getString() const -> std::string {
    return std::format("{} (*{})", displayName, filter);
  }
};

// TODO: refactor this back to immediate mode instead of retained mode.

class FileDialog {
public:
  FileDialog(std::shared_ptr<Preferences> newPreferences,
             std::vector<FilterItem> filterItems,
             std::string_view newUniqueName);
  ~FileDialog() = default;

  FileDialog(const FileDialog&) = delete;
  FileDialog(FileDialog&&) = delete;
  auto operator=(const FileDialog&) -> FileDialog& = delete;
  auto operator=(FileDialog&&) -> FileDialog& = delete;

  auto render() -> void;

  auto setOpen(std::optional<std::filesystem::path> newInitialPath = std::nullopt,
               std::optional<std::string> newLabel = std::nullopt) -> void;
  auto checkShouldOpen() -> void;

  auto setOnOk(const FileDialogOkFunction& fn) -> void;

  [[nodiscard]] auto getFinalSelection() const {
    return finalSelection;
  }

private:
  std::shared_ptr<Preferences> preferences;
  std::vector<FilterItem> filterItems;
  std::string uniqueName{};

  std::string lastPathKey;

  std::string label{(std::string(ICON_LC_FOLDER) + " Open File")};
  bool isOpen{};

  std::string selectedFilename;
  size_t selectedFilterIndex = 0;

  std::optional<std::filesystem::path> initialPath = std::nullopt;
  std::optional<std::filesystem::path> currentFolder;
  std::optional<std::filesystem::path> currentFile;

  std::optional<std::filesystem::path> finalSelection;

  ColumnInfo columnInfo;

  size_t folderSelectedIndex;
  size_t fileSelectedIndex;

  std::vector<FolderItem> folderItems;
  std::vector<FileItem> fileItems;

  std::optional<FileDialogOkFunction> onOk;
  std::optional<FileDialogCancelFunction> onCancel;

  auto updateFolderItems() -> void;
  auto updateFileItems() -> void;
};

}
