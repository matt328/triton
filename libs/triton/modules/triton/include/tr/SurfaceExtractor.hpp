#pragma once

#include "as/TerrainVertex.hpp"
#include "cm/TerrainResult.hpp"
#include "Transvoxel.hpp"

namespace tr {

class SdfGenerator;

constexpr std::array<glm::ivec3, 8> CornerIndex = {glm::vec3(0, 0, 0),
                                                   glm::vec3(1, 0, 0),
                                                   glm::vec3(0, 0, 1),
                                                   glm::vec3(1, 0, 1),
                                                   glm::vec3(0, 1, 0),
                                                   glm::vec3(1, 1, 0),
                                                   glm::vec3(0, 1, 1),
                                                   glm::vec3(1, 1, 1)};

class SurfaceExtractor {
public:
  SurfaceExtractor();
  ~SurfaceExtractor();

  SurfaceExtractor(const SurfaceExtractor&) = delete;
  SurfaceExtractor(SurfaceExtractor&&) = delete;
  auto operator=(const SurfaceExtractor&) -> SurfaceExtractor& = delete;
  auto operator=(SurfaceExtractor&&) -> SurfaceExtractor& = delete;

  auto extractSurface(const std::shared_ptr<SdfGenerator>& sdfGenerator,
                      SdfHandle sdfHandle,
                      const ChunkResult& chunk,
                      std::vector<as::TerrainVertex>& vertices,
                      std::vector<uint32_t>& indices) -> void;

private:
  RegularCellCache cache{0};

  auto extractCellVertices(const std::shared_ptr<SdfGenerator>& sdfGenerator,
                           SdfHandle sdfHandle,
                           glm::ivec3 min,
                           glm::ivec3 cellPosition,
                           std::vector<as::TerrainVertex>& vertices,
                           std::vector<uint32_t>& indices) -> void;

  auto generateVertex(std::vector<as::TerrainVertex>& vertices,
                      std::vector<as::TerrainVertex>& cellVertices,
                      glm::ivec3& offsetPosition,
                      glm::ivec3& cellPosition,
                      float t,
                      uint8_t corner0,
                      uint8_t corner1,
                      int8_t distance0,
                      int8_t distance1) -> int;
};
}
