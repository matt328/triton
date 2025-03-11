#include "DebugSurfaceExtractor.hpp"
#include "Transvoxel.hpp"
// NOLINTBEGIN
namespace tr {

auto DebugSurfaceExtractor::extractSurface(
    const std::shared_ptr<IDensityGenerator>& generator,
    const BlockResult& block,
    [[maybe_unused]] std::vector<as::TerrainVertex>& vertices,
    [[maybe_unused]] std::vector<uint32_t>& indices) -> void {
  Log.debug("DebugSurfaceExtractor extracting surface");

  CellCache cellCache{static_cast<size_t>(block.size.x)};

  auto worldBlockMin = glm::vec3(block.location.x * block.size.x,
                                 block.location.y * block.size.y,
                                 block.location.z * block.size.z);

  for (int yCoord = 0; yCoord < block.size.y - 1; ++yCoord) {
    for (int zCoord = 0; zCoord < block.size.z - 1; ++zCoord) {
      for (int xCoord = 0; xCoord < block.size.x - 1; ++xCoord) {
        auto blockCellPosition = glm::ivec3(xCoord, yCoord, zCoord);
        auto worldCellPosition = worldBlockMin + glm::vec3(blockCellPosition);
        extractCellVertices(generator,
                            worldCellPosition,
                            blockCellPosition,
                            cellCache,
                            vertices,
                            indices);
      }
    }
  }
}

auto DebugSurfaceExtractor::extractCellVertices(const std::shared_ptr<IDensityGenerator>& generator,
                                                glm::vec3 worldCellPosition,
                                                glm::ivec3 blockCellPosition,
                                                CellCache& cellCache,
                                                std::vector<as::TerrainVertex>& vertices,
                                                [[maybe_unused]] std::vector<uint32_t>& indices)
    -> void {
  // Sample the SDF's values at each corner of the current cell
  std::array<float, 8> corner{};
  for (int8_t currentCorner = 0; currentCorner < 8; ++currentCorner) {
    const auto cornerPosition = worldCellPosition + (CornerIndex[currentCorner]);
    corner[currentCorner] = generator->getValue(glm::vec3(cornerPosition));
  }

  /// The corner value in the SDF being non-negative means outside, negative means inside
  /// This code packs only the corner values' sign bits into a single 8 bit value which is how
  /// Lengyel's CellClass and CellData tables are indexed.
  uint8_t caseCode =
      ((corner[0] < 0 ? 0x01 : 0) | (corner[1] < 0 ? 0x02 : 0) | (corner[2] < 0 ? 0x04 : 0) |
       (corner[3] < 0 ? 0x08 : 0) | (corner[4] < 0 ? 0x10 : 0) | (corner[5] < 0 ? 0x20 : 0) |
       (corner[6] < 0 ? 0x40 : 0) | (corner[7] < 0 ? 0x80 : 0));

  // can bail on the whole cell right here with this check
  // 0 means the whole cell is either above or below, in either case, no verts are generated
  auto validCell = caseCode == 0 || caseCode == 255;
  if (!validCell) {
    return;
  }

  int8_t directionMask = (blockCellPosition.x > 0 ? 1 : 0) |
                         ((blockCellPosition.z > 0 ? 1 : 0) << 1) |
                         ((blockCellPosition.y > 0 ? 1 : 0) << 2);

  // Decode the data from Lengyel's tables
  auto equivalenceClassIndex = regularCellClass[caseCode];
  auto equivalenceClass = regularCellData[equivalenceClassIndex];

  const auto vertexCount = equivalenceClass.getVertexCount();
  [[maybe_unused]] const auto triangleCount = equivalenceClass.getTriangleCount();
  [[maybe_unused]] const auto vertexSequence = equivalenceClass.getVertexIndex();

  auto mappedIndices = std::vector<uint16_t>{};

  const auto vertexLocations = regularVertexData[caseCode];

  auto cellVertices = std::vector<as::TerrainVertex>{};

  for (uint8_t vli = 0; vli < vertexCount; ++vli) {
    auto vertexLocationInfo = vertexLocations[vli];
    uint8_t edgeInfo = highByte(vertexLocationInfo);

    /// Index of the vertex to be reused from the 'previous' cell.
    uint8_t reuseIndex = lowNibble(edgeInfo); // cacheIdx

    /// How to reach the preceeding cell.
    /// 0001 -> -x, 0010 -> -z 0100 -> -y, and 1000 -> no previous cell contains a reusable vertex
    uint8_t dirPrev = swapBits(highNibble(edgeInfo)); // cacheDir

    // Extract the indices of the corners of the cell to define the edge of the cell this vertex
    // lies on
    uint8_t cornerIndices = lowByte(vertexLocationInfo);

    uint8_t cornerIndex0 = lowNibble(cornerIndices);
    uint8_t cornerIndex1 = highNibble(cornerIndices);

    int8_t distance0 = corner[cornerIndex0];
    int8_t distance1 = corner[cornerIndex1];

    // If the value is on a surface, need to cache it in a consistent manner or else subsequent
    // cells might not find it? Still a little unclear on the details.
    if (distance1 == 0) {
      dirPrev = cornerIndex1 ^ 7;
      reuseIndex = 0;
    }

    if (distance0 == 0) {
      dirPrev = cornerIndex0 ^ 7;
      reuseIndex = 0;
    }

    const auto isCacheable = (dirPrev & directionMask) == dirPrev;

    int index = INVALID_INDEX;

    if (isCacheable) {
      const auto entry = cellCache.getReusedIndex(blockCellPosition, dirPrev);
      index = entry.vertices[reuseIndex];
    }

    if (!isCacheable || index == INVALID_INDEX) {
      index = generateVertex(vertices, reuseIndex);
    }
  }
}

auto DebugSurfaceExtractor::generateVertex(std::vector<as::TerrainVertex>& vertices,
                                           uint8_t reuseIndex) -> int {
  auto index = vertices.size();

  if (reuseIndex == 0) { // Vertex lies on an edge
  }

  return index;
}

}
// NOLINTEND
