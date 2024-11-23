#pragma once

#include <filesystem>
#include <tiny_gltf.h>

namespace tr::as {

   class NodeParser;
   class TransformParser;
   class GeometryExtractor;
   class TextureExtractor;
   class SkinningDataExtractor;
   class ModelLoader;
   class SkeletonLoader;
   struct Model;

   struct ModelResources {
      std::filesystem::path modelPath;
      std::optional<std::filesystem::path> skeletonPath = std::nullopt;
   };

   class ModelConverter {
    public:
      ModelConverter(std::unique_ptr<TransformParser> transformParser,
                     std::unique_ptr<GeometryExtractor> geometryExtractor,
                     std::unique_ptr<TextureExtractor> textureExtractor,
                     std::unique_ptr<SkinningDataExtractor> skinningDataExtractor,
                     std::unique_ptr<ModelLoader> modelLoader,
                     std::unique_ptr<SkeletonLoader> skeletonLoader);

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

      std::unique_ptr<NodeParser> nodeParser;
      std::unique_ptr<SkinningDataExtractor> skinningDataExtractor;
      std::unique_ptr<ModelLoader> modelLoader;
      std::unique_ptr<SkeletonLoader> skeletonLoader;
   };

} // namespace tr::as
