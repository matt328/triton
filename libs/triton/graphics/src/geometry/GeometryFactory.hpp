#pragma once

#include "as/Vertex.hpp"
#include "cm/Rando.hpp"
#include "HeightField.hpp"
#include "geometry/GeometryHandles.hpp"
#include "cm/sdf/DistanceField.hpp"
#include "geometry/Transvoxel.hpp"
#include <BaseException.hpp>

namespace tr::as {
   struct Model;
   class ImageData;
   struct Vertex;
} // namespace tr::as

namespace tr::ct {
   class HeightField;
} // namespace tr::ct

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

   constexpr size_t Size = 4;
   using VoxelArray = std::array<std::array<std::array<float, Size>, Size>, Size>;

   constexpr std::array<glm::ivec3, 8> CornerIndex = {glm::vec3(0, 0, 0),
                                                      glm::vec3(1, 0, 0),
                                                      glm::vec3(0, 0, 1),
                                                      glm::vec3(1, 0, 1),
                                                      glm::vec3(0, 1, 0),
                                                      glm::vec3(1, 1, 0),
                                                      glm::vec3(0, 1, 1),
                                                      glm::vec3(1, 1, 1)};

   class GeometryFactory {
    public:
      GeometryFactory();
      ~GeometryFactory();

      GeometryFactory(const GeometryFactory&) = delete;
      auto operator=(const GeometryFactory&) -> GeometryFactory& = delete;

      GeometryFactory(GeometryFactory&&) = delete;
      auto operator=(GeometryFactory&&) -> GeometryFactory& = delete;

      auto sdfPlane(const glm::vec3& point, const glm::ivec3& normal, float distance) -> int8_t;

      auto createTerrain() -> TexturedGeometryHandle;
      void polygonizeCell(glm::ivec3& offsetPosition,
                          glm::ivec3& cellPosition,
                          std::vector<as::Vertex>& vertices,
                          std::vector<uint32_t>& indices,
                          const VoxelArray& voxelData);

      auto generateVertex(std::vector<as::Vertex>& vertices,
                          glm::ivec3& offsetPosition,
                          glm::ivec3& cellPosition,
                          float t,
                          uint8_t corner0,
                          uint8_t corner1,
                          int8_t distance0,
                          int8_t distance1) -> int;

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
      RegularCellCache cache{Size * 10};

      cm::MapKey geometryKey{};
      cm::MapKey imageKey{};

      std::unordered_map<GeometryHandle, GeometryData> geometryDataMap;
      std::unordered_map<ImageHandle, as::ImageData> imageDataMap;

      static auto generateNormal(int xCoord, int yCoord, const ct::HeightField& heightField)
          -> glm::vec3;

      /// Loads a TRM file from the given path.
      /// @throws IOException if the file cannot be opened or parsed.
      static auto loadTrmFile(const std::filesystem::path& modelPath) -> as::Model;
   };
} // namespace tr::gfx::geo
