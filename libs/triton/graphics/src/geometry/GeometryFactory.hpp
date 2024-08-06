#pragma once

#include "cm/Rando.hpp"

#include "geometry/GeometryHandles.hpp"
#include "HeightField.hpp"

#include <BaseException.hpp>

namespace tr::as {
   struct Model;
   class ImageData;
}

namespace tr::ct {
   class HeightField;
}

namespace tr::gfx::geo {

   class GeometryData;

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

   class IOException final : public tr::BaseException {
    public:
      using BaseException::BaseException;
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

      auto createGeometryFromHeightfield(const ct::HeightField& heightField)
          -> TexturedGeometryHandle;

      /// Load a TRM file and cache its data
      /// @throws IOException if there's an error loading the file.
      auto loadTrm(const std::filesystem::path& modelPath) -> TritonModelData;

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

      static auto generateNormal(int x, int y, const ct::HeightField& heightField) -> glm::vec3;

      /// Loads a TRM file from the given path.
      /// @throws IOException if the file cannot be opened or parsed.
      static auto loadTrmFile(const std::string& modelPath) -> as::Model;
   };
}
