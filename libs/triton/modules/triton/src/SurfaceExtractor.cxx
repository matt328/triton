#include "tr/SurfaceExtractor.hpp"
#include "CellCache.hpp"
#include "tr/SdfGenerator.hpp"
#include "cm/GlmToString.hpp"
// NOLINTBEGIN
namespace tr {

SurfaceExtractor::SurfaceExtractor() {
}

SurfaceExtractor::~SurfaceExtractor() {
}

auto SurfaceExtractor::extractSurface(const std::shared_ptr<SdfGenerator>& sdfGenerator,
                                      SdfHandle sdfHandle,
                                      const BlockResult& chunk,
                                      std::vector<as::TerrainVertex>& vertices,
                                      std::vector<uint32_t>& indices) -> void {
  cache = CellCache{static_cast<size_t>(chunk.size.x)};
  /// World coordinates of the lower front left cell
  const auto voxelSize = sdfGenerator->getVoxelSize(sdfHandle);
  auto worldBlockMin = glm::vec3(chunk.location.x * chunk.size.x,
                                 chunk.location.y * chunk.size.y,
                                 chunk.location.z * chunk.size.z) *
                       voxelSize;
  ;

  // The algorithm starts from the lower front left, +x, then +z, creating a single 'layer' then
  // proceeds to the higher layers
  for (int yCoord = 0; yCoord < chunk.size.y - 1; ++yCoord) {
    for (int zCoord = 0; zCoord < chunk.size.z - 1; ++zCoord) {
      for (int xCoord = 0; xCoord < chunk.size.x - 1; ++xCoord) {
        auto blockCellPosition = glm::ivec3(xCoord, yCoord, zCoord);
        auto worldCellPosition = worldBlockMin + (glm::vec3(blockCellPosition) * voxelSize);
        extractCellVertices(sdfGenerator,
                            sdfHandle,
                            worldCellPosition,
                            blockCellPosition,
                            vertices,
                            indices);
      }
    }
  }
}

auto SurfaceExtractor::extractCellVertices(const std::shared_ptr<SdfGenerator>& sdfGenerator,
                                           SdfHandle sdfHandle,
                                           glm::vec3 worldCellPosition,
                                           glm::ivec3 blockCellPosition,
                                           std::vector<as::TerrainVertex>& vertices,
                                           std::vector<uint32_t>& indices) -> void {

  const auto voxelSize = sdfGenerator->getVoxelSize(sdfHandle);

  // Z and Y swapped here, need to swap dirPrev in order to stay consistent
  int8_t directionMask = (blockCellPosition.x > 0 ? 1 : 0) |
                         ((blockCellPosition.z > 0 ? 1 : 0) << 1) |
                         ((blockCellPosition.y > 0 ? 1 : 0) << 2);

  // Sample the SDF's values at each corner of the current cell
  std::array<int8_t, 8> corner{};
  for (int8_t currentCorner = 0; currentCorner < 8; ++currentCorner) {
    const auto cornerPosition = worldCellPosition + (CornerIndex[currentCorner] * voxelSize);
    const auto value = sdfGenerator->sampleSdf(sdfHandle, glm::vec3(cornerPosition));
    // assert(value != 0);
    corner[currentCorner] = value;
  }

  Log.debug("corner {},{},{},{},{},{},{},{}",
            corner[0],
            corner[1],
            corner[2],
            corner[3],
            corner[4],
            corner[5],
            corner[6],
            corner[7]);

  /// The corner value in the SDF being non-negative means outside, negative means inside
  /// This code packs only the corner values' sign bits into a single 8 bit value which is how
  /// Lengyel's CellClass and CellData tables are indexed.
  uint8_t caseCode = ((corner[0] >> 7) & 0x01) << 7 | // Corner 0
                     ((corner[1] >> 7) & 0x01) << 6 | // Corner 1
                     ((corner[2] >> 7) & 0x01) << 5 | // Corner 2
                     ((corner[3] >> 7) & 0x01) << 4 | // Corner 3
                     ((corner[4] >> 7) & 0x01) << 3 | // Corner 4
                     ((corner[5] >> 7) & 0x01) << 2 | // Corner 5
                     ((corner[6] >> 7) & 0x01) << 1 | // Corner 6
                     ((corner[7] >> 7) & 0x01);       // Corner 7

  // can bail on the whole cell right here with this check
  // 0 means the whole cell is either above or below, in either case, no verts are generated
  auto validCell = (caseCode ^ ((corner[7] >> 7) & 0xFF)) != 0;
  if (!validCell) {
    return;
  }
  Log.debug("corner {},{},{},{},{},{},{},{}",
            corner[0],
            corner[1],
            corner[2],
            corner[3],
            corner[4],
            corner[5],
            corner[6],
            corner[7]);
  Log.debug("Corner Position: {}", worldCellPosition + CornerIndex[0]);

  auto equivalenceClassIndex = regularCellClass[caseCode];
  auto equivalenceClass = regularCellData[equivalenceClassIndex];

  const auto vertexCount = equivalenceClass.getVertexCount();
  const auto triangleCount = equivalenceClass.getTriangleCount();
  const auto vertexSequence = equivalenceClass.getVertexIndex();
  auto mappedIndices = std::vector<uint16_t>{};

  const auto vertexLocations = regularVertexData[caseCode];

  auto cellVertices = std::vector<as::TerrainVertex>{};

  for (uint8_t vli = 0; vli < vertexCount; ++vli) {
    auto vertexLocationInfo = vertexLocations[vli];
    uint8_t edgeInfo = highByte(vertexLocationInfo);

    /// Index of the vertex to be reused from the 'previous' cell.
    uint8_t reuseIndex = lowNibble(edgeInfo);

    /// How to reach the preceeding cell.
    /// 0001 -> -x, 0010 -> -z 0100 -> -y, and 1000 -> no previous cell contains a reusable vertex
    uint8_t dirPrev = swapBits(highNibble(edgeInfo));

    // Extract the indices of the corners of the cell to define the edge of the cell this vertex
    // lies on
    uint8_t cornerIndices = lowByte(vertexLocationInfo);
    uint8_t cornerIndex0 = lowNibble(cornerIndices);
    uint8_t cornerIndex1 = highNibble(cornerIndices);

    Log.debug("Cell={} Edge={}-{}, reuseIndex={}, dirPrev={}",
              blockCellPosition,
              cornerIndex1,
              cornerIndex0,
              reuseIndex,
              std::bitset<8>(dirPrev).to_string());

    int8_t distance1 = corner[cornerIndex1];
    int8_t distance0 = corner[cornerIndex0];

    // Calculate t0 and t1, which are the fractional distances, scaled to [0,1] from each corner of
    // the y location of the surface
    int32_t t = (distance0 << 8) / (distance0 - distance1);
    int32_t u = 0x0100 - t; // compliment of t

    // scale each value into [0,1]
    float t0 = t / 256.F;
    [[maybe_unused]] float t1 = u / 256.F;

    int index = INVALID_INDEX;

    // If upperCellCornerIndex is 7, that means this cell must own the vertex, no previous cell
    // could have generated a vertex here as we have not visited any cells yet that have edges in
    // common with this edge
    const auto cellOwnsVertex = cornerIndex0 == 7;
    const auto previousCellExists = (dirPrev & directionMask) == dirPrev;

    // Check to see if a vertex has already been created on a previous cell's shared edge
    if (!cellOwnsVertex && previousCellExists) {

      const auto reuseCell = cache.getReusedIndex(blockCellPosition, dirPrev);
      index = reuseCell.vertices[reuseIndex];
      if (index != INVALID_INDEX) {
        Log.debug("Found reused vertex, cell={}: reuseIndex={}, index={}, dirPrev={}",
                  blockCellPosition,
                  reuseIndex,
                  index,
                  std::bitset<8>(dirPrev).to_string());
      } else {
        Log.warn("Expected but did not find cached vertex: cell={}: reuseIndex={}, index={}",
                 blockCellPosition,
                 reuseIndex,
                 index);
      }
    } else {
      Log.warn("Previous Cell did not exist");
    }

    // Did not find a previously generated vertex, so we must generate one
    if (index == INVALID_INDEX) {
      Log.debug("Generating Vertex for cell={}, edge={}-{}",
                blockCellPosition,
                cornerIndex1,
                cornerIndex0);
      index = generateVertex(vertices,
                             cellVertices,
                             worldCellPosition,
                             t0,
                             cornerIndex1,
                             cornerIndex0,
                             distance1,
                             distance0,
                             voxelSize);

      if (cornerIndex1 == 3) {
        Log.trace("setReusableIndex: Cell {}: reuseIndex: {}, index: {}",
                  blockCellPosition,
                  reuseIndex,
                  index);
        cache.setReusableIndex(blockCellPosition, reuseIndex, index);
      }
    }
    mappedIndices.push_back(index);
  }
  for (uint32_t t = 0; t < triangleCount; t++) {
    for (int i = 0; i < 3; ++i) {
      indices.push_back(mappedIndices[vertexSequence[t * 3 + i]]);
    }
  }
}

auto SurfaceExtractor::generateVertex(std::vector<as::TerrainVertex>& vertices,
                                      std::vector<as::TerrainVertex>& cellVertices,
                                      glm::vec3& offsetPosition,
                                      float t,
                                      uint8_t corner0,
                                      uint8_t corner1,
                                      [[maybe_unused]] int8_t distance0,
                                      [[maybe_unused]] int8_t distance1,
                                      float voxelSize) -> int {
  auto iP0 = (offsetPosition + CornerIndex[corner0] * voxelSize);
  auto P0 = glm::vec3(iP0);
  auto iP1 = (offsetPosition + CornerIndex[corner1] * voxelSize);
  auto P1 = glm::vec3(iP1);

  auto result = glm::mix(P0, P1, t);
  auto vertex = as::TerrainVertex{};
  vertex.position = result;
  auto size = vertices.size();
  vertices.push_back(vertex);

  cellVertices.push_back(vertex);

  return size;
}

}
// NOLINTEND
