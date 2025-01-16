#pragma once

#include <filesystem>
#include <tiny_gltf.h>

namespace as {

class INodeParser;
class ITransformParser;
class IGeometryExtractor;
class ITextureExtractor;
class ISkinningDataExtractor;
class IModelLoader;
class ISkeletonLoader;
struct Model;

struct ModelResources {
  std::filesystem::path modelPath;
  std::optional<std::filesystem::path> skeletonPath = std::nullopt;
};

class ModelConverter {
public:
  ModelConverter(std::unique_ptr<ITransformParser> transformParser,
                 std::unique_ptr<IGeometryExtractor> geometryExtractor,
                 std::unique_ptr<ITextureExtractor> textureExtractor,
                 std::unique_ptr<ISkinningDataExtractor> skinningDataExtractor,
                 std::unique_ptr<IModelLoader> modelLoader,
                 std::unique_ptr<ISkeletonLoader> skeletonLoader);

  ~ModelConverter() noexcept = default;

  ModelConverter(const ModelConverter&) = delete;
  auto operator=(const ModelConverter&) -> ModelConverter& = delete;

  ModelConverter(ModelConverter&&) = delete;
  auto operator=(ModelConverter&&) -> ModelConverter& = delete;

  void load(const ModelResources& resources);

  [[nodiscard]] auto buildTritonModel() const -> Model;

private:
  tinygltf::Model model;
  std::optional<ozz::animation::Skeleton> skeleton = std::nullopt;

  std::unique_ptr<INodeParser> nodeParser;
  std::unique_ptr<ISkinningDataExtractor> skinningDataExtractor;
  std::unique_ptr<IModelLoader> modelLoader;
  std::unique_ptr<ISkeletonLoader> skeletonLoader;
};

} // namespace as
