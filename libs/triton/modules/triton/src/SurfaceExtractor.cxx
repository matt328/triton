#include "tr/SurfaceExtractor.hpp"
#include "tr/SdfGenerator.hpp"

namespace tr {

SurfaceExtractor::SurfaceExtractor() {
}

SurfaceExtractor::~SurfaceExtractor() {
}

auto SurfaceExtractor::extractSurface(const std::shared_ptr<SdfGenerator>& sdfGenerator,
                                      SdfHandle sdfHandle,
                                      const ChunkResult& chunk,
                                      std::vector<as::TerrainVertex>& vertices,
                                      std::vector<uint32_t>& indices) -> void {
  /// World coordinates of the lower front left cell
  auto min = glm::ivec3(chunk.location.x * chunk.size.x,
                        chunk.location.y * chunk.size.y,
                        chunk.location.z * chunk.size.z);

  // The algorithm starts from the lower front left, +x, then +z, creating a single 'layer' then
  // proceeds to the higher layers
  for (int yCoord = 0; yCoord < chunk.size.y - 1; ++yCoord) {
    for (int zCoord = 0; zCoord < chunk.size.z - 1; ++zCoord) {
      for (int xCoord = 0; xCoord < chunk.size.x - 1; ++xCoord) {
        auto cellPosition = glm::ivec3(xCoord, yCoord, zCoord);
        extractCellVertices(sdfGenerator, sdfHandle, min, cellPosition, vertices, indices);
      }
    }
  }
}

auto SurfaceExtractor::extractCellVertices(const std::shared_ptr<SdfGenerator>& sdfGenerator,
                                           SdfHandle sdfHandle,
                                           glm::ivec3 min,
                                           glm::ivec3 cellPosition,
                                           std::vector<as::TerrainVertex>& vertices,
                                           std::vector<uint32_t>& indices) -> void {
  auto currentCellPosition = min + cellPosition;

  int8_t directionMask = (cellPosition.x > 0 ? 1 : 0) | ((cellPosition.z > 0 ? 1 : 0) << 1) |
                         ((cellPosition.y > 0 ? 1 : 0) << 2);

  // Sample the SDF's values at each corner of the current cell
  std::array<int8_t, 8> corner{};
  for (int8_t currentCorner = 0; currentCorner < 8; ++currentCorner) {
    const auto cornerPosition = currentCellPosition + CornerIndex[currentCorner];
    const auto value = sdfGenerator->sampleSdf(sdfHandle, cornerPosition);
    assert(value != 0);
    corner[currentCorner] = value;
  }

  // TODO(matt): given the sdf values at the corners, I don't think the case code is not being
  // generated correctly

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

  auto equivalenceClassIndex = regularCellClass[caseCode];
  auto equivalenceClass = regularCellData[equivalenceClassIndex];

  const auto vertexCount = equivalenceClass.getVertexCount();
  const auto triangleCount = equivalenceClass.getTriangleCount();
  const auto vertexSequence = equivalenceClass.getVertexIndex();
  auto mappedIndices = std::vector<uint16_t>{};

  const auto vertexLocations = regularVertexData[caseCode];

  auto cellVertices = std::vector<as::TerrainVertex>{};

  for (uint8_t vli = 0; vli < vertexCount; ++vli) {
    auto vertexLocation = vertexLocations[vli];
    /// Edge information is encoded in the high nibble of regularVertexData
    uint8_t edge = vertexLocation >> 8;
    /// Reuse information from the low nibble, this is the index of the vertex in the
    /// preceeding cell (cube) to use
    uint8_t reuseIndex = edge & 0xF;

    // Directions to the preceeding cell (cube) are in the high nibble.
    // bit value 1 = -x, bit value 2 = -y bit value 4 = -z bit value 8 means create new
    // vertex
    uint8_t dirPrev = edge >> 4;

    uint8_t cellCornerIndex1 = vertexLocation & 0x0F;
    uint8_t cellCornerIndex0 = (vertexLocation >> 4) & 0x0F;

    int8_t distance0 = corner[cellCornerIndex0];
    int8_t distance1 = corner[cellCornerIndex1];

    // Calculate distance from distance1 that the sign change occurs, ie the surface exists,
    // in the range of 0-256
    int32_t t = (distance1 << 8) / (distance1 - distance0);
    int32_t u = 0x0100 - t; // compliment of t

    // scale each value into [0,1]
    float t0 = t / 256.F;
    [[maybe_unused]] float t1 = u / 256.F;

    int index = -1;
    // If cellCornderIndex1 is 7, that means this cell does not own the vertex. And if the
    // dirPrev (which comes from the tables) & this edge's direction mask is itself, that
    // means the vertex in question could have already been generated, so we have to check.
    if (cellCornerIndex1 != 7 && (dirPrev & directionMask) == dirPrev) {
      const auto reuseCell = cache.getReusedIndex(cellPosition, dirPrev);
      index = reuseCell.vertices[reuseIndex];
    }

    if (index == -1) {
      // The cube in dirPrev did not generate a vertex on the edge we are needing one
      // so generate one.
      index = generateVertex(vertices,
                             cellVertices,
                             currentCellPosition,
                             cellPosition,
                             t0,
                             cellCornerIndex0,
                             cellCornerIndex1,
                             distance0,
                             distance1);
    }

    if ((dirPrev & 8) != 0) {
      // dirPrev having bit 8 set means create a new vertex
      uint16_t lastAddedVertexIndex = 0;
      cache.setReusableIndex(cellPosition, reuseIndex, lastAddedVertexIndex);
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
                                      glm::ivec3& offsetPosition,
                                      [[maybe_unused]] glm::ivec3& cellPosition,
                                      float t,
                                      uint8_t corner0,
                                      uint8_t corner1,
                                      [[maybe_unused]] int8_t distance0,
                                      [[maybe_unused]] int8_t distance1) -> int {
  auto iP0 = (offsetPosition + CornerIndex[corner0]);
  auto P0 = glm::vec3(iP0);
  auto iP1 = (offsetPosition + CornerIndex[corner1]);
  auto P1 = glm::vec3(iP1);

  auto result = glm::mix(P0, P1, t);
  auto vertex = as::TerrainVertex{};
  vertex.position = result;
  auto size = vertices.size();
  Log.debug("Vertex: {0}", vertex);
  vertices.push_back(vertex);

  cellVertices.push_back(vertex);

  return size;
}

}
