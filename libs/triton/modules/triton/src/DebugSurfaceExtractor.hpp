#pragma once

#include "CellCache.hpp"
#include "tr/ISurfaceExtractor.hpp"

namespace tr {

constexpr auto Padding = glm::ivec3(1, 1, 1);

struct BlockContext {
  std::shared_ptr<IDensityGenerator> generator;
  CellCache* cellCache;
  glm::vec3 worldCellPosition;
  glm::vec3 blockCellPosition;
  std::vector<as::TerrainVertex>& vertices;
  std::vector<uint32_t>& indices;
  glm::vec3 worldBlockMin;
  uint32_t blockSize;
  uint8_t lod;
  uint8_t lodScale;
};

struct VertexContext {
  uint8_t reuseIndex;
  float distance0;
  float distance1;
  uint8_t cornerIndex0;
  uint8_t cornerIndex1;
  bool isCacheable;
};

class DebugSurfaceExtractor : public ISurfaceExtractor {
public:
  DebugSurfaceExtractor() = default;

  auto extractSurface(const std::shared_ptr<IDensityGenerator>& generator,
                      const BlockResult& block,
                      std::vector<as::TerrainVertex>& vertices,
                      std::vector<uint32_t>& indices) -> void override;

private:
  auto extractCellVertices(BlockContext& ctx) -> void;

  auto generateVertex(const BlockContext& bCtx, VertexContext& vCtx) -> int;
};

}
