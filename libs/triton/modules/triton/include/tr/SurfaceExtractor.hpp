#pragma once

#include "as/TerrainVertex.hpp"
#include "cm/TerrainResult.hpp"
#include "Transvoxel.hpp"

namespace tr {

class SdfGenerator;

constexpr std::array<glm::vec3, 8> CornerIndex = {
    glm::vec3(0, 0, 0), // 0         6---------7
    glm::vec3(1, 0, 0), // 1        /|        /|
    glm::vec3(0, 0, 1), // 2	     / |       / |  Corners
    glm::vec3(1, 0, 1), // 3	    4---------5  |
    glm::vec3(0, 1, 0), // 4	    |  2------|--3
    glm::vec3(1, 1, 0), // 5	    | /       | /   y z
    glm::vec3(0, 1, 1), // 6	    |/        |/    |/
    glm::vec3(1, 1, 1)  // 7	    0---------1     o--x
};

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
                      const BlockResult& chunk,
                      std::vector<as::TerrainVertex>& vertices,
                      std::vector<uint32_t>& indices) -> void;

private:
  RegularCellCache cache{0};

  static auto swapBits(uint8_t x) -> uint8_t {
    uint8_t bit2 = (x >> 2) & 1; // Extract bit 2
    uint8_t bit3 = (x >> 3) & 1; // Extract bit 3

    // Clear bits 2 and 3 in x
    x &= ~(1 << 2);
    x &= ~(1 << 3);

    // Set bits 2 and 3 with swapped values
    x |= (bit2 << 3);
    x |= (bit3 << 2);

    return x;
  }

  static auto highByte(uint16_t input) -> uint8_t {
    return input >> 8;
  }

  static auto lowByte(uint16_t input) -> uint8_t {
    return input & 0xFF;
  }

  static auto lowNibble(uint8_t input) -> uint8_t {
    return input & 0xF;
  }

  static auto highNibble(uint8_t input) -> uint8_t {
    return input >> 4;
  }

  auto extractCellVertices(const std::shared_ptr<SdfGenerator>& sdfGenerator,
                           SdfHandle sdfHandle,
                           glm::vec3 worldCellPosition,
                           glm::ivec3 cellPosition,
                           std::vector<as::TerrainVertex>& vertices,
                           std::vector<uint32_t>& indices) -> void;

  auto generateVertex(std::vector<as::TerrainVertex>& vertices,
                      std::vector<as::TerrainVertex>& cellVertices,
                      glm::vec3& offsetPosition,
                      float t,
                      uint8_t corner0,
                      uint8_t corner1,
                      int8_t distance0,
                      int8_t distance1,
                      float voxelSize) -> int;
};
}
