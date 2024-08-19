#pragma once

#include <tiny_gltf.h>

namespace tr::as {
   struct Model;

   class TransformParser {
    public:
      virtual ~TransformParser() = default;
      [[nodiscard]] virtual glm::mat4 execute(const tinygltf::Node& node) const = 0;
   };

   class GeometryExtractor {
    public:
      virtual ~GeometryExtractor() = default;
      virtual void execute(const tinygltf::Model& model,
                           const tinygltf::Primitive& primitive,
                           const glm::mat4& transform,
                           Model& tritonModel) = 0;
   };

   class TextureExtractor {
    public:
      virtual ~TextureExtractor() = default;
      virtual void execute(const tinygltf::Model& model, int textureIndex, Model& tritonModel) = 0;
   };

   class SkinningDataExtractor {
    public:
      virtual ~SkinningDataExtractor() = default;
      virtual void execute(const tinygltf::Model& model,
                           const ozz::animation::Skeleton& skeleton,
                           Model& tritonModel) = 0;
   };

   class NodeParser {
    public:
      virtual ~NodeParser() = default;
      virtual void execute(const tinygltf::Model& model,
                           const tinygltf::Node& node,
                           Model& tritonModel) = 0;
   };

   class GltfFileLoader {
    public:
      virtual ~GltfFileLoader() = default;
      virtual bool loadFromFile(tinygltf::Model* model,
                                std::string* err,
                                std::string* warn,
                                const std::string& filename) = 0;
   };

   class ModelLoader {
    public:
      virtual ~ModelLoader() = default;

      virtual auto load(GltfFileLoader* loader, const std::filesystem::path& path) const
          -> tinygltf::Model = 0;
   };

   class SkeletonLoader {
    public:
      virtual ~SkeletonLoader() = default;
      virtual auto load(const std::filesystem::path& path) const -> ozz::animation::Skeleton = 0;
   };
}