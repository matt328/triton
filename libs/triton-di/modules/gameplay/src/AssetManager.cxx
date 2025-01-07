#include "gp/AssetManager.hpp"
#include "cm/Handles.hpp"
#include "geo/GeometryHandles.hpp"
#include "vk/VkResourceManager.hpp"
#include "as/Model.hpp"

#include "GlmCereal.hpp"

namespace tr {
AssetManager::AssetManager(std::shared_ptr<VkResourceManager> newResourceManager)
    : resourceManager{std::move(newResourceManager)} {
}

auto AssetManager::loadModel(std::string_view filename) -> ModelData {
  auto tritonModelData = loadTrm(filename);

  auto geometryHandle = tritonModelData.getGeometryHandle();
  auto imageHandle = tritonModelData.getImageHandle();

  auto geometryData = geometryDataMap.at(geometryHandle);

  auto meshHandle = resourceManager->asyncUpload2(geometryData);
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

auto AssetManager::createCube() -> ModelData {

  auto geometryHandle = generateAABB({-0.5f, -0.5f, -0.5f}, {0.5f, 0.5f, 0.5f});
  auto geometryData = geometryDataMap.at(geometryHandle);

  auto meshHandle = resourceManager->uploadStaticMesh(geometryData);
  geometryDataMap.erase(geometryHandle);

  return ModelData{
      .meshData =
          MeshData{.meshHandle = meshHandle, .topology = Topology::Triangles, .textureHandle = 0},
      .skinData = std::nullopt,
      .animationData = std::nullopt};
}

auto AssetManager::generateAABB(const glm::vec3& min, const glm::vec3& max) -> GeometryHandle {

  auto vertices = std::vector<as::Vertex>{{
      {.pos = {min.x, min.y, min.z}, .color = {1.f, 1.f, 1.f, 1.f}}, // 0: Bottom-left-back
      {.pos = {max.x, min.y, min.z}, .color = {0.f, 1.f, 1.f, 1.f}}, // 1: Bottom-right-back
      {.pos = {max.x, max.y, min.z}, .color = {1.f, 0.f, 1.f, 1.f}}, // 2: Top-right-back
      {.pos = {min.x, max.y, min.z}, .color = {1.f, 1.f, 0.f, 1.f}}, // 3: Top-left-back
      {.pos = {min.x, min.y, max.z}, .color = {1.f, 1.f, 1.f, 1.f}}, // 4: Bottom-left-front
      {.pos = {max.x, min.y, max.z}, .color = {1.f, 1.f, 1.f, 1.f}}, // 5: Bottom-right-front
      {.pos = {max.x, max.y, max.z}, .color = {1.f, 1.f, 1.f, 1.f}}, // 6: Top-right-front
      {.pos = {min.x, max.y, max.z}, .color = {1.f, 1.f, 1.f, 1.f}}  // 7: Top-left-front
  }};

  std::vector<uint32_t> indices = {// Back face
                                   0,
                                   1,
                                   2,
                                   2,
                                   3,
                                   0,
                                   // Front face
                                   4,
                                   5,
                                   6,
                                   6,
                                   7,
                                   4,
                                   // Left face
                                   0,
                                   4,
                                   7,
                                   7,
                                   3,
                                   0,
                                   // Right face
                                   1,
                                   5,
                                   6,
                                   6,
                                   2,
                                   1,
                                   // Bottom face
                                   0,
                                   1,
                                   5,
                                   5,
                                   4,
                                   0,
                                   // Top face
                                   3,
                                   2,
                                   6,
                                   6,
                                   7,
                                   3};

  const auto key = geometryKey.getKey();
  auto geometryHandle = GeometryHandle{key, Topology::LineList};
  geometryDataMap.emplace(geometryHandle, GeometryData{vertices, indices});
  return geometryHandle;
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
