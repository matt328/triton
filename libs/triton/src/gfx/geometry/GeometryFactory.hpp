#pragma once

#include "cm/Rando.hpp"

#include "GeometryHandles.hpp"
#include "ct/HeightField.hpp"

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
      GeometryFactory();
      ~GeometryFactory();

      GeometryFactory(const GeometryFactory&) = delete;
      GeometryFactory& operator=(const GeometryFactory&) = delete;

      GeometryFactory(GeometryFactory&&) = delete;
      GeometryFactory& operator=(GeometryFactory&&) = delete;

      auto createGeometryFromHeightfield(const ct::HeightField& heightfield)
          -> TexturedGeometryHandle;

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

      auto createGeometry(const tinygltf::Model& model,
                          const tinygltf::Primitive& primitive,
                          const glm::mat4& transform) -> GeometryHandle;
      auto generateNormal(int x, int y, const ct::HeightField& heightField) -> glm::vec3;
   };
}
