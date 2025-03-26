#pragma once

#include <filesystem>
#include <tiny_gltf.h>
#include "ConverterComponents.hpp"

namespace as {

class INodeParser;
class ITransformParser;
class IGeometryExtractor;
class ITextureExtractor;
class ISkinningDataExtractor;
class ISkeletonLoader;
struct Model;

struct ModelResources {
  std::filesystem::path modelPath;
  std::optional<std::filesystem::path> skeletonPath = std::nullopt;
  std::filesystem::path outputFile;
};

class ModelConverter {
public:
  ModelConverter(std::unique_ptr<ITransformParser> transformParser,
                 std::unique_ptr<IGeometryExtractor> geometryExtractor,
                 std::unique_ptr<ITextureExtractor> textureExtractor,
                 std::unique_ptr<ISkinningDataExtractor> skinningDataExtractor,
                 std::unique_ptr<IModelLoader<tinygltf::Model>> modelLoader,
                 std::unique_ptr<ISkeletonLoader> skeletonLoader);

  ~ModelConverter() noexcept = default;

  ModelConverter(const ModelConverter&) = delete;
  auto operator=(const ModelConverter&) -> ModelConverter& = delete;

  ModelConverter(ModelConverter&&) = delete;
  auto operator=(ModelConverter&&) -> ModelConverter& = delete;

  auto load(const std::filesystem::path& path) -> void;
  [[nodiscard]] auto hasSkin() const -> bool;
  auto addSkeleton(const std::filesystem::path& path) -> void;
  void load(const ModelResources& resources);
  [[nodiscard]] auto isReady() const -> bool;

  [[nodiscard]] auto buildTritonModel() const -> Model;

private:
  tinygltf::Model model;
  std::optional<ozz::animation::Skeleton> skeleton = std::nullopt;

  std::unique_ptr<INodeParser> nodeParser;
  std::unique_ptr<ISkinningDataExtractor> skinningDataExtractor;
  std::unique_ptr<IModelLoader<tinygltf::Model>> modelLoader;
  std::unique_ptr<ISkeletonLoader> skeletonLoader;
};

}
