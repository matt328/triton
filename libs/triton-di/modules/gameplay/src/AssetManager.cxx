#include "gp/AssetManager.hpp"
#include "geo/GeometryHandles.hpp"
#include "vk/VkResourceManager.hpp"
#include "as/Model.hpp"

#include "GlmCereal.hpp"
#include <optional>

namespace tr {
AssetManager::AssetManager(std::shared_ptr<VkResourceManager> newResourceManager)
    : resourceManager{std::move(newResourceManager)} {
}

auto AssetManager::loadModel(std::string_view filename) -> ModelData {
  auto tritonModelData = loadTrm(filename);

  auto geometryHandle = tritonModelData.getGeometryHandle();
  auto imageHandle = tritonModelData.getImageHandle();

  auto geometryData = geometryDataMap.at(geometryHandle);

  auto meshHandle = resourceManager->asyncUpload(geometryData);
  auto textureHandle = resourceManager->uploadImage(imageDataMap.at(imageHandle));

  geometryDataMap.erase(geometryHandle);
  imageDataMap.erase(imageHandle);

  return ModelData{.meshData =
                       MeshData{
                           .meshHandle = meshHandle,
                           .topology = Topology::Triangles,
                           .textureHandle = textureHandle,
                       },
                   .skinData = std::nullopt,
                   .animationData = std::nullopt};
}

/// Loads a TRM model.
/// @throws IOException if there's an issue opening or reading the file.
auto AssetManager::loadTrm(const std::filesystem::path& modelPath) -> TritonModelData {

  Log.debug("Loading TRM File: {0}", modelPath.string());

  auto tritonModel = loadTrmFile(modelPath.string());

  Log.debug("Joint Remaps Size: {0}", tritonModel.jointRemaps.size());
  for (const auto& [position, sortedIndex] : tritonModel.jointRemaps) {
    Log.debug("Joint Remap: {0}, {1}", position, sortedIndex);
  }

  Log.debug("inverseBindPoses.size(): {0}", tritonModel.inverseBindPoses.size());

  const auto imageHandle = imageKey.getKey();
  imageDataMap.emplace(imageHandle, tritonModel.imageData);

  const auto key = geometryKey.getKey();
  const auto geometryHandle = GeometryHandle{key, Topology::Triangles};
  geometryDataMap.emplace(geometryHandle, GeometryData{tritonModel.vertices, tritonModel.indices});

  auto skinData = std::optional<SkinData>{};
  if (tritonModel.skinned()) {
    skinData = SkinData(tritonModel.jointRemaps, tritonModel.inverseBindPoses);
  }

  return TritonModelData{geometryHandle, imageHandle, skinData};
}

auto AssetManager::loadTrmFile(const std::filesystem::path& modelPath) -> as::Model {
  try {
    ZoneNamedN(z, "Loading Model from Disk", true);
    if (modelPath.extension() == ".trm") {
      auto is = std::ifstream(modelPath, std::ios::binary);
      if (!is) {
        throw IOException("Failed to open file: " + modelPath.string());
      }

      try {
        cereal::PortableBinaryInputArchive input(is);
        auto tritonModel = as::Model{};
        input(tritonModel);
        return tritonModel;
      } catch (const std::exception& ex) {
        throw IOException("Error reading: " + modelPath.string() + ": ", ex);
      }
    } else if (modelPath.extension() == ".json") {
      auto is = std::ifstream(modelPath);
      if (!is) {
        throw IOException("Failed to open file: " + modelPath.string());
      }
      try {
        cereal::JSONInputArchive input(is);
        auto tritonModel = as::Model{};
        input(tritonModel);
        return tritonModel;
      } catch (const std::exception& ex) {
        throw IOException("Error reading: " + modelPath.string() + ": ", ex);
      }
    }
  } catch (BaseException& ex) {
    ex << "GeometryFactory::loadTrmFile(): ";
    throw;
  }
  Log.warn("loadTrmFile returning empty model");
  return {};
}

}
