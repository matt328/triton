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

   class GeometryDataNotFoundException : public std::logic_error {
    public:
      explicit GeometryDataNotFoundException(const std::string& message)
          : std::logic_error(message) {
      }
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

      auto loadTrm(const std::filesystem::path& modelPath) -> std::optional<TritonModelData>;

      void unload(const TexturedGeometryHandle& handle);

      /// Gets the GeometryData from the internal cache
      /// @throws GeometryDataNotFoundException if no data in the cache matches the given handle.
      [[nodiscard]] auto getGeometryData(const GeometryHandle& handle) -> GeometryData;

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
