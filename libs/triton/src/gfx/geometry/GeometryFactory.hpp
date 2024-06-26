#pragma once

#include "GeometryHandles.hpp"
#include "ct/HeightField.hpp"
#include "gfx/geometry/AnimationFactory.hpp"
#include "gfx/helpers/Rando.hpp"

namespace tr::ct {
   class HeightField;
}

namespace tr::gfx::geo {

   class GeometryData;
   class ImageData;

   struct GltfNode {
      int index;
      int number;
      std::string name;
   };

   class GeometryFactory {
    public:
      GeometryFactory(AnimationFactory& animationFactory);
      ~GeometryFactory();

      GeometryFactory(const GeometryFactory&) = delete;
      GeometryFactory& operator=(const GeometryFactory&) = delete;

      GeometryFactory(GeometryFactory&&) = delete;
      GeometryFactory& operator=(GeometryFactory&&) = delete;

      auto createGeometryFromHeightfield(const ct::HeightField& heightfield)
          -> TexturedGeometryHandle;

      auto loadGeometryFromGltf(const std::filesystem::path& filename) -> TexturedGeometryHandle;

      auto loadAnimatedGeometryFromGltf(const std::filesystem::path& filename,
                                        const SkeletonHandle& skeletonHandle)
          -> SkinnedGeometryData;

      auto loadSkinnedModel(const std::filesystem::path& modelPath,
                            const std::filesystem::path& skeletonPath,
                            const std::filesystem::path& animationPath) -> SkinnedGeometryData;

      void unload(const TexturedGeometryHandle& handle);

      [[nodiscard]] auto getGeometryData(const GeometryHandle& handle) -> GeometryData&;

      [[nodiscard]] auto getImageData(const ImageHandle& handle) -> ImageData&;

    private:
      rng::MapKey geometryKey{};
      rng::MapKey imageKey{};

      std::unordered_map<GeometryHandle, GeometryData> geometryDataMap;
      std::unordered_map<ImageHandle, ImageData> imageDataMap;

      AnimationFactory& animationFactory;

      auto createGeometry(const tinygltf::Model& model,
                          const tinygltf::Primitive& primitive,
                          const glm::mat4& transform) -> GeometryHandle;
      auto generateNormal(int x, int y, const ct::HeightField& heightField) -> glm::vec3;
      auto createTexture(const tinygltf::Model& model, std::size_t textureIndex) -> ImageHandle;

      auto parseNodeTransform(const tinygltf::Node& node) -> glm::mat4;

      auto parseNode(const tinygltf::Model& model,
                     const tinygltf::Node& node,
                     std::unordered_map<int, ImageHandle>& loadedTextureIndices,
                     TexturedGeometryHandle& handle) -> void;
   };
}
