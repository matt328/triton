#include "AssetTool.hpp"
#include "as/Model.hpp"
#include "as/ModelConverter.hpp"
#include "as/gltf/GltfGeometryExtractor.hpp"
#include "as/gltf/GltfModelLoader.hpp"
#include "as/gltf/GltfSkeletonLoader.hpp"
#include "as/gltf/GltfSkinningDataExtractor.hpp"
#include "as/gltf/GltfTextureExtractor.hpp"
#include "as/gltf/GltfTransformParser.hpp"
#include "imgui.h"
#include "ui/assets/IconsLucide.hpp"

#include <di.hpp>

namespace di = boost::di;

namespace ed {

const auto InputFilters = std::vector{FilterItem{.filter = ".gltf", .displayName = "GLTF File"},
                                      FilterItem{.filter = ".*", .displayName = "All Files"}};
const auto SkeletonFilters =
    std::vector{FilterItem{.filter = ".ozz", .displayName = "Ozz Skeleton File"},
                FilterItem{.filter = ".*", .displayName = "All Files"}};

const auto TrmFilters =
    std::vector{FilterItem{.filter = ".trm", .displayName = "Triton Model File"},
                FilterItem{.filter = ".*", .displayName = "All Files"}};

AssetTool::AssetTool(std::shared_ptr<Preferences> preferences) {
  Log.trace("Constructing Asset Tool");
  const auto injector =
      di::make_injector(di::bind<as::ITransformParser>.to<as::GltfTransformParser>(),
                        di::bind<as::IGeometryExtractor>.to<as::GltfGeometryExtractor>(),
                        di::bind<as::ITextureExtractor>.to<as::GltfTextureExtractor>(),
                        di::bind<as::ISkinningDataExtractor>.to<as::GltfSkinningDataExtractor>(),
                        di::bind<as::IModelLoader<tinygltf::Model>>.to<as::GltfModelLoader>(),
                        di::bind<as::ISkeletonLoader>.to<as::GltfSkeletonLoader>());

  modelConverter = injector.create<std::shared_ptr<as::ModelConverter>>();

  inputFileDialog = std::make_unique<FileDialog>(preferences, InputFilters, "assettool-gltf");
  inputFileDialog->setOnOk([&](std::vector<std::filesystem::path> selectedFile) {
    try {
      inputFile = selectedFile.front();
      modelConverter->load(inputFile);
    } catch (const std::exception& ex) { Log.error(ex.what()); }
  });

  inputSkeletonDialog =
      std::make_unique<FileDialog>(preferences, SkeletonFilters, "assettool-skeleton");
  inputSkeletonDialog->setOnOk([&](std::vector<std::filesystem::path> selectedFile) {
    try {
      inputFile = selectedFile.front();
      modelConverter->addSkeleton(inputFile);
    } catch (const std::exception& ex) { Log.error(ex.what()); }
  });

  outputFileDialog = std::make_unique<FileDialog>(preferences, TrmFilters, "assettool-trm");
  outputFileDialog->setOnOk([&](std::vector<std::filesystem::path> selectedFile) {
    try {
      auto outputFile = selectedFile.front();
      const auto model = modelConverter->buildTritonModel();
      if (!outputFile.has_extension()) {
        outputFile.replace_extension("trm");
      }
      writeOutputFile(outputFile, model);
    } catch (const std::exception& ex) { Log.error(ex.what()); }
  });
}

AssetTool::~AssetTool() {
  Log.trace("Destroying AssetTool");
}

auto AssetTool::render() -> void {
  if (ImGui::Begin(ComponentName, nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {

    if (ImGui::Button(ICON_LC_FOLDER_OPEN " Model", ImVec2(ImGuiConstants::ButtonWidth, 0.f))) {
      inputFileDialog->setOpen(std::nullopt, "Open GLTF File");
    }

    ImGui::BeginDisabled(!modelConverter->hasSkin());
    if (ImGui::Button(ICON_LC_FOLDER_OPEN " Skeleton", ImVec2(ImGuiConstants::ButtonWidth, 0.f))) {
      inputSkeletonDialog->setOpen(std::nullopt, "Open Skeleton");
    }
    ImGui::EndDisabled();

    ImGui::BeginDisabled(!modelConverter->isReady());
    if (ImGui::Button(ICON_LC_FOLDER_OPEN " TRM", ImVec2(ImGuiConstants::ButtonWidth, 0.f))) {
      outputFileDialog->setOpen(std::nullopt, "Save Triton Model");
    }
    ImGui::EndDisabled();
  }
  ImGui::End();

  inputFileDialog->checkShouldOpen();
  inputFileDialog->render();

  inputSkeletonDialog->checkShouldOpen();
  inputSkeletonDialog->render();

  outputFileDialog->checkShouldOpen();
  outputFileDialog->render();
}

auto AssetTool::writeOutputFile(const std::filesystem::path& outputFile,
                                const as::Model& tritonModel) -> bool {

  std::ofstream outputStream(outputFile, std::ios::binary);
  if (!outputStream) {
    return false;
  }

  cereal::PortableBinaryOutputArchive binOutput(outputStream);
  binOutput(tritonModel);
  outputStream.close();
  Log.info("Wrote binary output file to {0}", outputFile.string());
  return true;
}

}
