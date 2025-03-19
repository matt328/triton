#include "AssetManager.hpp"
#include "cm/Handles.hpp"
#include "geo/GeometryHandles.hpp"
#include "geo/DynamicGeometryData.hpp"
#include "geo/StaticGeometryData.hpp"
#include "vk/VkResourceManager.hpp"
#include "as/Model.hpp"

namespace tr {
AssetManager::AssetManager(std::shared_ptr<VkResourceManager> newResourceManager)
    : resourceManager{std::move(newResourceManager)} {
}

auto AssetManager::loadModel(std::string_view filename) -> ModelData {
  auto tritonModelData = loadTrm(filename);

  auto geometryHandle = tritonModelData.getGeometryHandle();
  auto imageHandle = tritonModelData.getImageHandle();

  auto& geometryData = geometryDataMap.at(geometryHandle);

  auto meshHandle = MeshHandle{};
  if (tritonModelData.getSkinData().has_value()) {
    meshHandle = resourceManager->uploadDynamicMesh(*geometryData);
  } else {
    meshHandle = resourceManager->uploadStaticMesh(*geometryData);
  }

  auto textureHandle = resourceManager->uploadImage(imageDataMap.at(imageHandle), "ModelTexture");

  geometryDataMap.erase(geometryHandle);
  imageDataMap.erase(imageHandle);

  // Animation Data?

  return ModelData{.meshData =
                       MeshData{
                           .meshHandle = meshHandle,
                           .topology = Topology::Triangles,
                           .textureHandle = textureHandle,
                       },
                   .skinData = tritonModelData.getSkinData(),
                   .animationData = std::nullopt};
}

auto AssetManager::loadSkeleton(std::string_view filename) -> SkeletonHandle {

  if (const auto it = loadedSkeletons.find(filename.data()); it != loadedSkeletons.end()) {
    return it->second;
  }

  ozz::io::File file(filename.data(), "rb");

  if (!file.opened()) {
    Log.error("Failed to open skeleton file: {0}.", filename);
  }

  ozz::io::IArchive archive(&file);
  if (!archive.TestTag<ozz::animation::Skeleton>()) {
    Log.error("Failed to load skeleton instance from file: {0}", filename);
  }

  auto skeleton = ozz::animation::Skeleton{};
  archive >> skeleton;

  const auto key = skeletonKey.getKey();

  skeletons.emplace(key, std::move(skeleton));

  loadedSkeletons.emplace(filename, key);

  return key;
}

auto AssetManager::loadAnimation(std::string_view filename) -> AnimationHandle {
  if (loadedAnimations.contains(filename.data())) {
    return loadedAnimations.at(filename.data());
  }

  ozz::io::File file(filename.data(), "rb");
  if (!file.opened()) {
    Log.error("Failed to open animation file: {0}", filename);
  }

  ozz::io::IArchive archive(&file);
  if (!archive.TestTag<ozz::animation::Animation>()) {
    Log.error("Failed to load animation from file: {0}", filename);
  }

  auto animation = ozz::animation::Animation{};
  archive >> animation;

  const auto key = animationKey.getKey();
  animations.emplace(key, std::move(animation));

  loadedAnimations.emplace(filename, key);

  return key;
}

auto AssetManager::getAnimation(AnimationHandle handle) const -> const ozz::animation::Animation& {
  return animations.at(handle);
}

auto AssetManager::getSkeleton(SkeletonHandle handle) const -> const ozz::animation::Skeleton& {
  return skeletons.at(handle);
}

auto AssetManager::createCube() -> ModelData {

  auto geometryHandle = generateAABB({-0.5f, -0.5f, -0.5f}, {0.5f, 0.5f, 0.5f});
  auto& geometryData = geometryDataMap.at(geometryHandle);

  auto meshHandle = resourceManager->uploadStaticMesh(*geometryData);
  geometryDataMap.erase(geometryHandle);

  return ModelData{.meshData = MeshData{.meshHandle = meshHandle,
                                        .topology = Topology::Triangles,
                                        .textureHandle = 1234},
                   .skinData = std::nullopt,
                   .animationData = std::nullopt};
}

auto AssetManager::generateAABB([[maybe_unused]] const glm::vec3& min,
                                [[maybe_unused]] const glm::vec3& max) -> GeometryHandle {

  // auto vertices = std::vector<as::Vertex>{{{
  //                                              // 0
  //                                              .pos = {min.x, min.y, min.z},
  //                                              .color = {0.f, 0.f, 1.f, 1.f},
  //                                          },
  //                                          {
  //                                              // 1
  //                                              .pos = {max.x, min.y, min.z},
  //                                              .color = {1.f, 1.f, 0.f, 1.f},
  //                                          },
  //                                          {
  //                                              // 2
  //                                              .pos = {max.x, max.y, min.z},
  //                                              .color = {0.f, 1.f, 0.f, 1.f},
  //                                          },
  //                                          {
  //                                              // 3
  //                                              .pos = {min.x, max.y, min.z},
  //                                              .color = {1.f, 0.f, 0.f, 1.f},
  //                                          },
  //                                          {
  //                                              // 4
  //                                              .pos = {min.x, min.y, max.z},
  //                                              .color = {0.f, 1.f, 1.f, 1.f},
  //                                          },
  //                                          {
  //                                              // 5
  //                                              .pos = {max.x, min.y, max.z},
  //                                              .color = {1.f, 0.f, 1.f, 1.f},
  //                                          },
  //                                          {
  //                                              // 6
  //                                              .pos = {max.x, max.y, max.z},
  //                                              .color = {1.f, 1.f, 1.f, 1.f},
  //                                          },
  //                                          {
  //                                              // 7
  //                                              .pos = {min.x, max.y, max.z},
  //                                              .color = {0.f, 0.f, 0.f, 1.f},
  //                                          }}};

  std::vector<uint32_t> indices = {0, 2, 1, 0, 3, 2, 0, 4, 3, 4, 7, 3, 4, 5, 7, 5, 6, 7,
                                   5, 1, 6, 1, 2, 6, 1, 5, 4, 1, 4, 0, 7, 2, 3, 7, 6, 2};

  const auto key = geometryKey.getKey();
  auto geometryHandle = GeometryHandle{key, Topology::LineList};
  // geometryDataMap.emplace(geometryHandle, GeometryData{vertices, indices});
  return geometryHandle;
}

/// Loads a TRM model.
/// @throws IOException if there's an issue opening or reading the file.
auto AssetManager::loadTrm(const std::filesystem::path& modelPath) -> TritonModelData {

  Log.debug("Loading TRM File: {0}", modelPath.string());

  auto tritonModel = loadTrmFile(modelPath.string());

  const auto imageHandle = imageKey.getKey();
  imageDataMap.emplace(imageHandle, tritonModel.imageData);

  auto geometryHandle = GeometryHandle{};
  if (tritonModel.dynamicVertices.has_value()) {
    const auto key = geometryKey.getKey();
    geometryHandle = GeometryHandle{.handle = key, .topology = Topology::Triangles};
    geometryDataMap.emplace(
        geometryHandle,
        std::make_unique<DynamicGeometryData>(std::move(tritonModel.dynamicVertices.value()),
                                              std::move(tritonModel.indices)));
  } else if (tritonModel.staticVertices.has_value()) {
    const auto key = geometryKey.getKey();
    geometryHandle = GeometryHandle{key, Topology::Triangles};
    geometryDataMap.emplace(
        geometryHandle,
        std::make_unique<StaticGeometryData>(std::move(tritonModel.staticVertices.value()),
                                             std::move(tritonModel.indices)));
  } else {
    throw std::runtime_error("Model either has to be dynamic or static");
  }

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
        is.close();
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
