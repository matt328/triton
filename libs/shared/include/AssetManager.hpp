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

class GeometryDataNotFoundException : public std::logic_error {
public:
  explicit GeometryDataNotFoundException(const std::string& message) : std::logic_error(message) {
  }
};

class IOException final : public tr::BaseException {
public:
  using BaseException::BaseException;
};

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

  /// Load a model from disk and return a handle to the mesh data.
  /// The handle will be used to identify the geometry needed to render the model to the renderer.
  /// This method will block until the geometry data is available to be rendered, so it should be
  /// called from a background thread. A lot of vulkan synchronization issues can be avoided by
  /// calling this method from a background thread, and simply not asking the renderer to render it
  /// until after the call is completed. Any vulkan operations that have to happen during this
  /// method will happen 'asynchronously' as far as Vulkan is concerned, meaning they will not block
  /// any rendering operations.
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
