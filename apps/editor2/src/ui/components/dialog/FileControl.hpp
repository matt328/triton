#pragma once

#include "Properties.hpp"
#include "ui/components/FileDialog.hpp"
#include "ui/components/dialog/ControlBase.hpp"

namespace ed {

class FileDialog;

class FileControl : public ControlBase {
public:
  explicit FileControl(std::string_view newLabel,
                       std::shared_ptr<Properties> newProperties,
                       const std::vector<FilterItem>& filterItems);
  ~FileControl() override;

  FileControl(const FileControl&) = delete;
  FileControl(FileControl&&) = delete;
  auto operator=(const FileControl&) -> FileControl& = delete;
  auto operator=(FileControl&&) -> FileControl& = delete;

  auto render() -> void override;
  [[nodiscard]] auto getValue() const -> std::any override;

private:
  std::string label;
  std::unique_ptr<FileDialog> fileDialog;

  std::filesystem::path value;
};

}
