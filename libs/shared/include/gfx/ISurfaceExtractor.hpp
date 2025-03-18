#pragma once

#include "cm/TerrainResult.hpp"
#include "tr/IDensityGenerator.hpp"
#include "as/TerrainVertex.hpp"

namespace tr {

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

class ISurfaceExtractor {
public:
  ISurfaceExtractor() = default;
  virtual ~ISurfaceExtractor() = default;

  ISurfaceExtractor(const ISurfaceExtractor&) = default;
  ISurfaceExtractor(ISurfaceExtractor&&) = delete;
  auto operator=(const ISurfaceExtractor&) -> ISurfaceExtractor& = default;
  auto operator=(ISurfaceExtractor&&) -> ISurfaceExtractor& = delete;

  virtual auto extractSurface(const std::shared_ptr<IDensityGenerator>& generator,
                              const BlockResult& block,
                              std::vector<as::TerrainVertex>& vertices,
                              std::vector<uint32_t>& indices) -> void = 0;

  static auto swapBits(uint8_t x) -> uint8_t {
    uint8_t bit2 = (x >> 2u) & 1u; // Extract bit 2
    uint8_t bit3 = (x >> 3u) & 1u; // Extract bit 3

    // Clear bits 2 and 3 in x
    x &= ~(1u << 2u);
    x &= ~(1u << 3u);

    // Set bits 2 and 3 with swapped values
    x |= (bit2 << 3u);
    x |= (bit3 << 2u);

    return x;
  }

  static auto highByte(uint16_t input) -> uint8_t {
    return input >> 8u;
  }

  static auto lowByte(uint16_t input) -> uint8_t {
    return input & 0xFF;
  }

  static auto lowNibble(uint8_t input) -> uint8_t {
    return input & 0xF;
  }

  static auto highNibble(uint8_t input) -> uint8_t {
    return input >> 4u;
  }
};

}
