#pragma once

#include "CellCache.hpp"
#include "tr/ISurfaceExtractor.hpp"

namespace tr {

class DebugSurfaceExtractor : public ISurfaceExtractor {
public:
  DebugSurfaceExtractor() = default;

  auto extractSurface(const std::shared_ptr<IDensityGenerator>& generator,
                      const BlockResult& block,
                      std::vector<as::TerrainVertex>& vertices,
                      std::vector<uint32_t>& indices) -> void override;

private:
  auto extractCellVertices(const std::shared_ptr<IDensityGenerator>& generator,
                           glm::vec3 worldCellPosition,
                           glm::ivec3 blockCellPosition,
                           CellCache& cellCache,
                           std::vector<as::TerrainVertex>& vertices,
                           std::vector<uint32_t>& indices) -> void;

  auto generateVertex(std::vector<as::TerrainVertex>& vertices, uint8_t reuseIndex) -> int;
};

}
