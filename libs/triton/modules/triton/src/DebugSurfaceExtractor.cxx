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

        auto ctx = BlockContext{.generator = generator,
                                .cellCache = &cellCache,
                                .worldCellPosition = worldCellPosition,
                                .blockCellPosition = blockCellPosition,
                                .vertices = vertices,
                                .indices = indices,
                                .worldBlockMin = worldBlockMin,
                                .blockSize = static_cast<uint32_t>(block.size.x),
                                .lod = 1,
                                .lodScale = 1 << 1};

        extractCellVertices(ctx);
      }
    }
  }
}

auto DebugSurfaceExtractor::extractCellVertices(BlockContext& ctx) -> void {
  // Sample the SDF's values at each corner of the current cell
  std::array<float, 8> corner{};
  for (int8_t currentCorner = 0; currentCorner < 8; ++currentCorner) {
    const auto cornerPosition = ctx.worldCellPosition + (CornerIndex[currentCorner]);
    corner[currentCorner] = ctx.generator->getValue(glm::vec3(cornerPosition));
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
  auto validCell = caseCode != 0 && caseCode != 255;
  if (!validCell) {
    return;
  }

  int8_t directionMask = (ctx.blockCellPosition.x > 0 ? 1 : 0) |
                         ((ctx.blockCellPosition.z > 0 ? 1 : 0) << 1) |
                         ((ctx.blockCellPosition.y > 0 ? 1 : 0) << 2);

  // Decode the data from Lengyel's tables
  auto equivalenceClassIndex = regularCellClass[caseCode];
  auto equivalenceClass = regularCellData[equivalenceClassIndex];

  const auto vertexCount = equivalenceClass.getVertexCount();
  const auto triangleCount = equivalenceClass.getTriangleCount();
  const auto vertexSequence = equivalenceClass.getVertexIndex();

  auto mappedIndices = std::vector<uint16_t>{};

  const auto vertexLocations = regularVertexData[caseCode];

  auto cellVertices = std::vector<as::TerrainVertex>{};
  auto cellIndices = std::vector<uint32_t>{};

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

    auto distance0 = corner[cornerIndex0];
    auto distance1 = corner[cornerIndex1];

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
      const auto entry = ctx.cellCache->getReusedIndex(ctx.blockCellPosition, dirPrev);
      index = entry.vertices[reuseIndex];
    }

    if (!isCacheable || index == INVALID_INDEX) {
      auto vCtx = VertexContext{
          .reuseIndex = reuseIndex,
          .distance0 = distance0,
          .distance1 = distance1,
          .cornerIndex0 = cornerIndex0,
          .cornerIndex1 = cornerIndex1,
          .isCacheable = isCacheable,
      };
      index = generateVertex(ctx, vCtx);
      cellIndices.push_back(index);
    }
  }

  auto indexCount = triangleCount * 3;
  for (uint32_t i = 0; i < indexCount; i += 3) {
    auto ia = cellIndices[vertexSequence[i + 0]];
    auto ib = cellIndices[vertexSequence[i + 1]];
    auto ic = cellIndices[vertexSequence[i + 2]];

    ctx.indices.push_back(ia);
    ctx.indices.push_back(ib);
    ctx.indices.push_back(ic);
  }
}

auto DebugSurfaceExtractor::generateVertex(const BlockContext& bCtx, VertexContext& vCtx) -> int {
  auto index = bCtx.vertices.size();
  int lod = 1;
  auto vertexPosition = glm::vec3{};
  auto normal = glm::vec3{};
  uint32_t vertBoundaryMask = 0;

  if (vCtx.reuseIndex == 0) { // Vertex lies on an edge
    glm::ivec3 cornerOffset =
        vCtx.distance0 == 0 ? CornerIndex[vCtx.cornerIndex0] : CornerIndex[vCtx.cornerIndex1];

    vertexPosition = bCtx.worldCellPosition + glm::vec3(cornerOffset);

    if (bCtx.lod > 0) {
      vertBoundaryMask =
          ((vertexPosition.x == 0 ? 1 : 0) | (vertexPosition.y == 0 ? 2 : 0) |
           (vertexPosition.z == 0 ? 4 : 0) | (vertexPosition.x == bCtx.blockSize ? 8 : 0) |
           (vertexPosition.y == bCtx.blockSize ? 16 : 0) |
           (vertexPosition.z == bCtx.blockSize ? 32 : 0));
    }

    vertexPosition += Padding;

    {
      const auto x = vertexPosition.x;
      const auto y = vertexPosition.y;
      const auto z = vertexPosition.z;
      normal =
          glm::vec3(bCtx.generator->getValue(x - 1, y, z) - bCtx.generator->getValue(x + 1, y, z),
                    bCtx.generator->getValue(x, y - 1, z) - bCtx.generator->getValue(x, y + 1, z),
                    bCtx.generator->getValue(x, y, z - 1) - bCtx.generator->getValue(x, y, z + 1));
    }

    bCtx.vertices.push_back(
        as::TerrainVertex{.position = vertexPosition, .texCoord = glm::ivec2(0, 0)});

    if (vCtx.isCacheable) {
      bCtx.cellCache->setReusableIndex(bCtx.blockCellPosition, vCtx.reuseIndex, index);
    }
  } else {
    auto vertexLocalPos0 = bCtx.blockCellPosition + CornerIndex[vCtx.cornerIndex0];
    auto vertexLocalPos1 = bCtx.blockCellPosition + CornerIndex[vCtx.cornerIndex1];

    auto vertexLocalPos0Float = glm::vec3(vertexLocalPos0);
    auto vertexLocalPos1Float = glm::vec3(vertexLocalPos1);

    for (int i = 0; i < lod; ++i) {
      auto midPointLocal = (vertexLocalPos0Float + vertexLocalPos1Float) * 0.5f;
      auto midPointWorld = bCtx.worldBlockMin + midPointLocal * static_cast<float>(bCtx.lodScale);
      auto midpointDistance = bCtx.generator->getValue(midPointWorld);

      if (std::signbit(vCtx.distance0) == std::signbit(midpointDistance)) {
        vertexLocalPos0Float = midPointLocal;
        vCtx.distance0 = midpointDistance;
      } else {
        vertexLocalPos1Float = midPointLocal;
        vCtx.distance1 = midpointDistance;
      }
    }

    auto t0 = vCtx.distance1 / (vCtx.distance1 - vCtx.distance0);
    auto t1 = 1 - t0;

    vertexPosition = vertexLocalPos0Float * t0 + vertexLocalPos1Float * t1;

    auto vertPosX0 = vertexLocalPos0.x;
    auto vertPosY0 = vertexLocalPos0.y;
    auto vertPosZ0 = vertexLocalPos0.z;

    auto vertPosX1 = vertexLocalPos1.x;
    auto vertPosY1 = vertexLocalPos1.y;
    auto vertPosZ1 = vertexLocalPos1.z;

    if (bCtx.lod > 0) {
      vertBoundaryMask = (((vertPosX0 == 0 || vertPosX1 == 0) ? 1 : 0) |
                          ((vertPosY0 == 0 || vertPosY1 == 0) ? 2 : 0) |
                          ((vertPosZ0 == 0 || vertPosZ1 == 0) ? 4 : 0) |
                          ((vertPosX0 == bCtx.blockSize || vertPosX1 == bCtx.blockSize) ? 8 : 0) |
                          ((vertPosY0 == bCtx.blockSize || vertPosY1 == bCtx.blockSize) ? 16 : 0) |
                          ((vertPosZ0 == bCtx.blockSize || vertPosZ1 == bCtx.blockSize) ? 32 : 0));
    }

    vertPosX0 += Padding.x;
    vertPosY0 += Padding.y;
    vertPosZ0 += Padding.z;

    vertPosX1 += Padding.x;
    vertPosY1 += Padding.y;
    vertPosZ1 += Padding.z;

    auto normal0 = glm::vec3(bCtx.generator->getValue(vertPosX0 - 1, vertPosY0, vertPosZ0) -
                                 bCtx.generator->getValue(vertPosX0 + 1, vertPosY0, vertPosZ0),
                             bCtx.generator->getValue(vertPosX0, vertPosY0 - 1, vertPosZ0) -
                                 bCtx.generator->getValue(vertPosX0, vertPosY0 + 1, vertPosZ0),
                             bCtx.generator->getValue(vertPosX0, vertPosY0, vertPosZ0 - 1) -
                                 bCtx.generator->getValue(vertPosX0, vertPosY0, vertPosZ0 + 1));

    auto normal1 = glm::vec3(bCtx.generator->getValue(vertPosX1 - 1, vertPosY1, vertPosZ1) -
                                 bCtx.generator->getValue(vertPosX1 + 1, vertPosY1, vertPosZ1),
                             bCtx.generator->getValue(vertPosX1, vertPosY1 - 1, vertPosZ1) -
                                 bCtx.generator->getValue(vertPosX1, vertPosY1 + 1, vertPosZ1),
                             bCtx.generator->getValue(vertPosX1, vertPosY1, vertPosZ1 - 1) -
                                 bCtx.generator->getValue(vertPosX1, vertPosY1, vertPosZ1 + 1));

    normal = normal0 + normal1;

    if (vCtx.cornerIndex1 == 7) {
      bCtx.cellCache->setReusableIndex(bCtx.blockCellPosition, vCtx.reuseIndex, index);
    }
  }

  normal = glm::normalize(normal);

  if (vertBoundaryMask > 0) {}

  bCtx.vertices.push_back(
      as::TerrainVertex{.position = vertexPosition * static_cast<float>(bCtx.lodScale),
                        .texCoord = glm::ivec2{0, 0}});

  return index;
}

}
// NOLINTEND
