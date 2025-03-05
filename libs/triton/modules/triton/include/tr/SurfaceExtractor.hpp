#pragma once

#include "as/TerrainVertex.hpp"

namespace tr {

class SdfGenerator;

class SurfaceExtractor {
public:
  SurfaceExtractor();
  ~SurfaceExtractor();

  SurfaceExtractor(const SurfaceExtractor&) = delete;
  SurfaceExtractor(SurfaceExtractor&&) = delete;
  auto operator=(const SurfaceExtractor&) -> SurfaceExtractor& = delete;
  auto operator=(SurfaceExtractor&&) -> SurfaceExtractor& = delete;

  auto extractSurface(const std::shared_ptr<SdfGenerator>& sdfGenerator,
                      glm::ivec3& offsetPosition,
                      glm::ivec3& cellPosition,
                      std::vector<as::TerrainVertex>& vertices,
                      std::vector<uint32_t>& indices) -> void;
};
}
