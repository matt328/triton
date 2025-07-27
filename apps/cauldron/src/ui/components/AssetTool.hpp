#pragma once

#include "as/ModelConverter.hpp"
#include "ui/components/FileDialog.hpp"

namespace ed {

class AssetTool {
public:
  explicit AssetTool(std::shared_ptr<Properties> properties);
  ~AssetTool();

  AssetTool(const AssetTool&) = delete;
  AssetTool(AssetTool&&) = delete;
  auto operator=(const AssetTool&) -> AssetTool& = delete;
  auto operator=(AssetTool&&) -> AssetTool& = delete;

  void render();

  static constexpr auto ComponentName = "Asset Tool";

private:
  std::unique_ptr<FileDialog> inputFileDialog;
  std::unique_ptr<FileDialog> inputSkeletonDialog;
  std::unique_ptr<FileDialog> outputFileDialog;
  std::filesystem::path inputFile;

  std::shared_ptr<as::ModelConverter> modelConverter;

  static auto writeOutputFile(const std::filesystem::path& outputFile, const as::Model& tritonModel)
      -> bool;
};

}
