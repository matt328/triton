#pragma once

#include <tiny_gltf.h>

namespace as {
struct Model;

class ITransformParser {
public:
  ITransformParser() = default;
  ITransformParser(const ITransformParser&) = delete;
  ITransformParser(ITransformParser&&) = delete;
  auto operator=(const ITransformParser&) -> ITransformParser& = delete;
  auto operator=(ITransformParser&&) -> ITransformParser& = delete;

  virtual ~ITransformParser() = default;
  [[nodiscard]] virtual auto execute(const tinygltf::Node& node) const -> glm::mat4 = 0;
};

class IGeometryExtractor {
public:
  IGeometryExtractor() = default;
  IGeometryExtractor(const IGeometryExtractor&) = delete;
  IGeometryExtractor(IGeometryExtractor&&) = delete;
  auto operator=(const IGeometryExtractor&) -> IGeometryExtractor& = delete;
  auto operator=(IGeometryExtractor&&) -> IGeometryExtractor& = delete;

  virtual ~IGeometryExtractor() = default;
  virtual void execute(const tinygltf::Model& model,
                       const tinygltf::Primitive& primitive,
                       const glm::mat4& transform,
                       Model& tritonModel) = 0;
};

class ITextureExtractor {
public:
  ITextureExtractor() = default;
  ITextureExtractor(const ITextureExtractor&) = delete;
  ITextureExtractor(ITextureExtractor&&) = delete;
  auto operator=(const ITextureExtractor&) -> ITextureExtractor& = delete;
  auto operator=(ITextureExtractor&&) -> ITextureExtractor& = delete;
  virtual ~ITextureExtractor() = default;
  virtual void execute(const tinygltf::Model& model, int textureIndex, Model& tritonModel) = 0;
};

class ISkinningDataExtractor {
public:
  ISkinningDataExtractor() = default;
  ISkinningDataExtractor(const ISkinningDataExtractor&) = delete;
  ISkinningDataExtractor(ISkinningDataExtractor&&) = delete;
  auto operator=(const ISkinningDataExtractor&) -> ISkinningDataExtractor& = delete;
  auto operator=(ISkinningDataExtractor&&) -> ISkinningDataExtractor& = delete;
  virtual ~ISkinningDataExtractor() = default;
  virtual void execute(const tinygltf::Model& model,
                       const ozz::animation::Skeleton& skeleton,
                       Model& tritonModel) = 0;
};

class INodeParser {
public:
  INodeParser() = default;
  INodeParser(const INodeParser&) = delete;
  INodeParser(INodeParser&&) = delete;
  auto operator=(const INodeParser&) -> INodeParser& = delete;
  auto operator=(INodeParser&&) -> INodeParser& = delete;
  virtual ~INodeParser() = default;
  virtual void execute(const tinygltf::Model& model,
                       const tinygltf::Node& node,
                       Model& tritonModel) = 0;
};

class IGltfFileLoader {
public:
  IGltfFileLoader() = default;
  IGltfFileLoader(const IGltfFileLoader&) = delete;
  IGltfFileLoader(IGltfFileLoader&&) = delete;
  auto operator=(const IGltfFileLoader&) -> IGltfFileLoader& = delete;
  auto operator=(IGltfFileLoader&&) -> IGltfFileLoader& = delete;
  virtual ~IGltfFileLoader() = default;
  virtual auto loadFromFile(tinygltf::Model* model,
                            std::string* err,
                            std::string* warn,
                            const std::string& filename) -> bool = 0;
};

class IModelLoader {
public:
  IModelLoader() = default;
  IModelLoader(const IModelLoader&) = delete;
  IModelLoader(IModelLoader&&) = delete;
  auto operator=(const IModelLoader&) -> IModelLoader& = delete;
  auto operator=(IModelLoader&&) -> IModelLoader& = delete;
  virtual ~IModelLoader() = default;

  virtual auto load(IGltfFileLoader* loader, const std::filesystem::path& path) const
      -> tinygltf::Model = 0;
};

class ISkeletonLoader {
public:
  ISkeletonLoader() = default;
  ISkeletonLoader(const ISkeletonLoader&) = delete;
  ISkeletonLoader(ISkeletonLoader&&) = delete;
  auto operator=(const ISkeletonLoader&) -> ISkeletonLoader& = delete;
  auto operator=(ISkeletonLoader&&) -> ISkeletonLoader& = delete;
  virtual ~ISkeletonLoader() = default;
  [[nodiscard]] virtual auto load(const std::filesystem::path& path) const
      -> ozz::animation::Skeleton = 0;
};
} // namespace as
