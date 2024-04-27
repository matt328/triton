#pragma once

#include "GeometryHandles.hpp"
#include "ct/HeightField.hpp"
#include <random>

namespace tr::ct {
   class HeightField;
}

namespace tr::gfx::geo {

   class GeometryData;
   class ImageData;

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
      auto loadGeometryFromGltf(const std::filesystem::path& filename) -> TexturedGeometryHandle;

      void unload(const TexturedGeometryHandle& handle);

      [[nodiscard]] auto getGeometryData(const GeometryHandle& handle) -> GeometryData&;

      [[nodiscard]] auto getImageData(const ImageHandle& handle) -> ImageData&;

    private:
      std::random_device imageRandomDevice;
      std::mt19937 imageGen;
      std::uniform_int_distribution<std::size_t> imageDistribution;

      std::random_device geometryRandomDevice;
      std::mt19937 geometryGen;
      std::uniform_int_distribution<std::size_t> geometryDistribution;

      std::unordered_map<GeometryHandle, GeometryData> geometryDataMap;
      std::unordered_map<ImageHandle, ImageData> imageDataMap;

      auto createGeometry(const tinygltf::Model& model, const tinygltf::Primitive& primitive)
          -> GeometryHandle;
      auto generateNormal(int x, int y, const ct::HeightField& heightField) -> glm::vec3;
      auto createTexture(const tinygltf::Model& model, std::size_t textureIndex) -> ImageHandle;

      auto parseNode(const tinygltf::Model& model,
                     const tinygltf::Node& node,
                     std::unordered_map<int, ImageHandle>& loadedTextureIndices,
                     TexturedGeometryHandle& handle) -> void;
   };
}
