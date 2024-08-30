#pragma once

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
      ModelConverter& operator=(const ModelConverter&) = delete;

      ModelConverter(ModelConverter&&) = delete;
      ModelConverter& operator=(ModelConverter&&) = delete;

      void load(const std::filesystem::path& modelPath, const std::filesystem::path& skeletonPath);

      [[nodiscard]] auto buildTritonModel() const -> Model;

    private:
      tinygltf::Model model;
      ozz::animation::Skeleton skeleton;

      std::unique_ptr<NodeParser> nodeParser;
      std::unique_ptr<SkinningDataExtractor> skinningDataExtractor;
      std::unique_ptr<ModelLoader> modelLoader;
      std::unique_ptr<SkeletonLoader> skeletonLoader;
   };

}
