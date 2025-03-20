#pragma once

#include "cm/Handles.hpp"
#include "cm/Rando.hpp"
#include "cm/GeometryHandles.hpp"
#include "as/Model.hpp"
#include <ozz/animation/runtime/skeleton.h>

namespace as {
struct Model;
}

namespace tr {

class IGeometryData;
struct TritonModelData;
class VkResourceManager;

class AssetManager {
public:
  explicit AssetManager(std::shared_ptr<VkResourceManager> newResourceManager);
  ~AssetManager() = default;

  AssetManager(const AssetManager&) = delete;
  AssetManager(AssetManager&&) = delete;
  auto operator=(const AssetManager&) -> AssetManager& = delete;
  auto operator=(AssetManager&&) -> AssetManager& = delete;

  auto loadModel(std::string_view filename) -> ModelData;

  auto loadSkeleton(std::string_view filename) -> SkeletonHandle;

  auto loadAnimation(std::string_view filename) -> AnimationHandle;

  [[nodiscard]] auto getAnimation(AnimationHandle handle) const -> const ozz::animation::Animation&;
  [[nodiscard]] auto getSkeleton(SkeletonHandle handle) const -> const ozz::animation::Skeleton&;

  auto createCube() -> ModelData;

private:
  std::shared_ptr<VkResourceManager> resourceManager;

  MapKey geometryKey{};
  MapKey imageKey{};
  MapKey skeletonKey{};
  MapKey animationKey{};

  std::unordered_map<GeometryHandle, std::unique_ptr<IGeometryData>> geometryDataMap;
  std::unordered_map<ImageHandle, as::ImageData> imageDataMap;

  std::unordered_map<std::string, AnimationHandle> loadedAnimations;
  std::unordered_map<std::string, SkeletonHandle> loadedSkeletons;

  std::unordered_map<AnimationHandle, ozz::animation::Animation> animations;
  std::unordered_map<SkeletonHandle, ozz::animation::Skeleton> skeletons;

  /// Load a TRM file and cache its data
  /// @throws IOException if there's an error loading the file.
  auto loadTrm(const std::filesystem::path& modelPath) -> TritonModelData;
  /// Loads a TRM file from the given path.
  /// @throws IOException if the file cannot be opened or parsed.
  static auto loadTrmFile(const std::filesystem::path& modelPath) -> as::Model;

  auto generateAABB(const glm::vec3& min, const glm::vec3& max) -> GeometryHandle;
};

}
