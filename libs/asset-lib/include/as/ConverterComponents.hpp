#pragma once

#include <tiny_gltf.h>

namespace tr::as {
   struct Model;

   class TransformParser {
    public:
      TransformParser() = default;
      TransformParser(const TransformParser&) = delete;
      TransformParser(TransformParser&&) = delete;
      auto operator=(const TransformParser&) -> TransformParser& = delete;
      auto operator=(TransformParser&&) -> TransformParser& = delete;

      virtual ~TransformParser() = default;
      [[nodiscard]] virtual glm::mat4 execute(const tinygltf::Node& node) const = 0;
   };

   class GeometryExtractor {
    public:
      GeometryExtractor() = default;
      GeometryExtractor(const GeometryExtractor&) = delete;
      GeometryExtractor(GeometryExtractor&&) = delete;
      auto operator=(const GeometryExtractor&) -> GeometryExtractor& = delete;
      auto operator=(GeometryExtractor&&) -> GeometryExtractor& = delete;

      virtual ~GeometryExtractor() = default;
      virtual void execute(const tinygltf::Model& model,
                           const tinygltf::Primitive& primitive,
                           const glm::mat4& transform,
                           Model& tritonModel) = 0;
   };

   class TextureExtractor {
    public:
      TextureExtractor() = default;
      TextureExtractor(const TextureExtractor&) = delete;
      TextureExtractor(TextureExtractor&&) = delete;
      auto operator=(const TextureExtractor&) -> TextureExtractor& = delete;
      auto operator=(TextureExtractor&&) -> TextureExtractor& = delete;
      virtual ~TextureExtractor() = default;
      virtual void execute(const tinygltf::Model& model, int textureIndex, Model& tritonModel) = 0;
   };

   class SkinningDataExtractor {
    public:
      SkinningDataExtractor() = default;
      SkinningDataExtractor(const SkinningDataExtractor&) = delete;
      SkinningDataExtractor(SkinningDataExtractor&&) = delete;
      auto operator=(const SkinningDataExtractor&) -> SkinningDataExtractor& = delete;
      auto operator=(SkinningDataExtractor&&) -> SkinningDataExtractor& = delete;
      virtual ~SkinningDataExtractor() = default;
      virtual void execute(const tinygltf::Model& model,
                           const ozz::animation::Skeleton& skeleton,
                           Model& tritonModel) = 0;
   };

   class NodeParser {
    public:
      NodeParser() = default;
      NodeParser(const NodeParser&) = delete;
      NodeParser(NodeParser&&) = delete;
      auto operator=(const NodeParser&) -> NodeParser& = delete;
      auto operator=(NodeParser&&) -> NodeParser& = delete;
      virtual ~NodeParser() = default;
      virtual void execute(const tinygltf::Model& model,
                           const tinygltf::Node& node,
                           Model& tritonModel) = 0;
   };

   class GltfFileLoader {
    public:
      GltfFileLoader() = default;
      GltfFileLoader(const GltfFileLoader&) = delete;
      GltfFileLoader(GltfFileLoader&&) = delete;
      auto operator=(const GltfFileLoader&) -> GltfFileLoader& = delete;
      auto operator=(GltfFileLoader&&) -> GltfFileLoader& = delete;
      virtual ~GltfFileLoader() = default;
      virtual auto loadFromFile(tinygltf::Model* model,
                                std::string* err,
                                std::string* warn,
                                const std::string& filename) -> bool = 0;
   };

   class ModelLoader {
    public:
      ModelLoader() = default;
      ModelLoader(const ModelLoader&) = delete;
      ModelLoader(ModelLoader&&) = delete;
      auto operator=(const ModelLoader&) -> ModelLoader& = delete;
      auto operator=(ModelLoader&&) -> ModelLoader& = delete;
      virtual ~ModelLoader() = default;

      virtual auto load(GltfFileLoader* loader, const std::filesystem::path& path) const
          -> tinygltf::Model = 0;
   };

   class SkeletonLoader {
    public:
      SkeletonLoader() = default;
      SkeletonLoader(const SkeletonLoader&) = delete;
      SkeletonLoader(SkeletonLoader&&) = delete;
      auto operator=(const SkeletonLoader&) -> SkeletonLoader& = delete;
      auto operator=(SkeletonLoader&&) -> SkeletonLoader& = delete;
      virtual ~SkeletonLoader() = default;
      [[nodiscard]] virtual auto load(const std::filesystem::path& path) const
          -> ozz::animation::Skeleton = 0;
   };
} // namespace tr::as