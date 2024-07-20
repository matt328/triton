#pragma once

#include "cm/Rando.hpp"

#include "geometry/GeometryHandles.hpp"
#include "HeightField.hpp"

namespace tr::as {
   struct Model;
   class ImageData;
}

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

   using GeometryDataRef = std::optional<std::reference_wrapper<GeometryData>>;

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

      auto loadTrm(const std::filesystem::path& modelPath) -> TexturedGeometryHandle;

      auto loadSkinnedModel(const std::filesystem::path& modelPath,
                            const std::filesystem::path& skeletonPath,
                            const std::filesystem::path& animationPath) -> SkinnedGeometryData;

      void unload(const TexturedGeometryHandle& handle);

      [[nodiscard]] auto getGeometryData(const GeometryHandle& handle) -> GeometryDataRef;

      [[nodiscard]] auto getImageData(const ImageHandle& handle) -> as::ImageData&;

    private:
      cm::MapKey geometryKey{};
      cm::MapKey imageKey{};

      std::unordered_map<GeometryHandle, GeometryData> geometryDataMap;
      std::unordered_map<ImageHandle, as::ImageData> imageDataMap;

      auto generateNormal(int x, int y, const ct::HeightField& heightField) -> glm::vec3;
      auto loadTrmFile(const std::string& modelPath) -> std::optional<as::Model>;
   };
}
