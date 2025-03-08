#include "tr/SurfaceExtractor.hpp"
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
                                      const ChunkResult& chunk,
                                      std::vector<as::TerrainVertex>& vertices,
                                      std::vector<uint32_t>& indices) -> void {
  cache = RegularCellCache{static_cast<size_t>(chunk.size.x)};
  /// World coordinates of the lower front left cell
  auto min = glm::ivec3(chunk.location.x * chunk.size.x,
                        chunk.location.y * chunk.size.y,
                        chunk.location.z * chunk.size.z);

  /* TODO(matt):
    P17 of the pdf shows we need to have access to one more cell in each of the 3 directions, even
    though we aren't triangulating these cells, they have to be there so we can know where the
    vertices lie on the edges

    so make sure to populate the cell grid
  */

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

  // Z and Y swapped here, need to swap dirPrev in order to stay consistent
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

  /* TODO(matt): given the sdf values at the corners, I don't think the case code is not being
    Ehh I think the case code is being generated correctly, just the 8th vertex, the second vertex
    of the second cell has a value of 0,y,0, instead of 1,y,1.
  */

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
    auto vertexLocationInfo = vertexLocations[vli];
    uint8_t edgeInfo = highByte(vertexLocationInfo);

    /// Index of the vertex to be reused from the 'previous' cell.
    uint8_t reuseIndex = lowNibble(edgeInfo);

    // Todo(matt) the table data swaps y and z. Need to account for this in the dirPrev

    /// How to reach the preceeding cell.
    /// 0001 -> -x, 0010 -> -z 0100 -> -y, and 1000 -> no previous cell contains a reusable vertex
    uint8_t dirPrev = swapBits(highNibble(edgeInfo));

    // Extract the indices of the corners of the cell to define the edge of the cell this vertex
    // lies on
    uint8_t cornerIndices = lowByte(vertexLocationInfo);
    uint8_t upperCellCornerIndex = lowNibble(cornerIndices);
    uint8_t lowerCellCornerIndex = highNibble(cornerIndices);

    Log.debug("Cell={} Edge={}-{}, reuseIndex={}, dirPrev={}",
              cellPosition,
              lowerCellCornerIndex,
              upperCellCornerIndex,
              reuseIndex,
              std::bitset<8>(dirPrev).to_string());

    int8_t lowerDistanceToSurface = corner[lowerCellCornerIndex];
    int8_t upperDistanceToSurface = corner[upperCellCornerIndex];

    // Calculate t0 and t1, which are the fractional distances, scaled to [0,1] from each corner of
    // the y location of the surface
    int32_t t = (upperDistanceToSurface << 8) / (upperDistanceToSurface - lowerDistanceToSurface);
    int32_t u = 0x0100 - t; // compliment of t

    // scale each value into [0,1]
    float t0 = t / 256.F;
    [[maybe_unused]] float t1 = u / 256.F;

    int index = INVALID_INDEX;

    // If upperCellCornerIndex is 7, that means this cell must own the vertex, no previous cell
    // could have generated a vertex here as we have not visited any cells yet that have edges in
    // common with this edge
    const auto cellOwnsVertex = upperCellCornerIndex == 7;
    const auto previousCellExists = (dirPrev & directionMask) == dirPrev;

    // Check to see if a vertex has already been created on a previous cell's shared edge
    if (!cellOwnsVertex && previousCellExists) {

      const auto reuseCell = cache.getReusedIndex(cellPosition, dirPrev);
      index = reuseCell.vertices[reuseIndex];
      if (index != INVALID_INDEX) {
        Log.debug("Found reused vertex, cell={}: reuseIndex={}, index={}, dirPrev={}",
                  cellPosition,
                  reuseIndex,
                  index,
                  std::bitset<8>(dirPrev).to_string());
      } else {
        Log.warn("Expected but did not find cached vertex: cell={}: reuseIndex={}, index={}",
                 cellPosition,
                 reuseIndex,
                 index);
      }
    } else {
      Log.warn("Previous Cell did not exist");
    }

    // Did not find a previously generated vertex, so we must generate one
    if (index == INVALID_INDEX) {
      Log.debug("Generating Vertex for cell={}, edge={}-{}",
                cellPosition,
                lowerCellCornerIndex,
                upperCellCornerIndex);
      index = generateVertex(vertices,
                             cellVertices,
                             currentCellPosition,
                             cellPosition,
                             t0,
                             lowerCellCornerIndex,
                             upperCellCornerIndex,
                             lowerDistanceToSurface,
                             upperDistanceToSurface);

      if (lowerCellCornerIndex == 3) {
        Log.trace("setReusableIndex: Cell {}: reuseIndex: {}, index: {}",
                  cellPosition,
                  reuseIndex,
                  index);
        cache.setReusableIndex(cellPosition, reuseIndex, index);
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
  vertices.push_back(vertex);

  cellVertices.push_back(vertex);

  return size;
}

}
// NOLINTEND
