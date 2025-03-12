#pragma once

namespace tr {
static constexpr uint16_t INVALID_INDEX = std::numeric_limits<uint16_t>::max();

struct CellCacheEntry {
  std::vector<uint16_t> vertices;
  explicit CellCacheEntry(size_t size) {
    vertices.resize(size, INVALID_INDEX);
  }
};

class CellCache {
public:
  explicit CellCache(size_t chunkSize) : chunkSize(chunkSize) {
    cache = {std::vector<CellCacheEntry>{}, std::vector<CellCacheEntry>{}};
    for (size_t size = 0; size < chunkSize * chunkSize; ++size) {
      cache[0].emplace_back(4);
      cache[1].emplace_back(4);
    }
  }

  auto getReusedIndex(glm::ivec3 pos, uint8_t dir) -> CellCacheEntry {
    size_t rx = dir & 0x01u;
    size_t rz = (static_cast<size_t>(dir) >> 1u) & 0x01u;
    size_t ry = (static_cast<size_t>(dir) >> 2u) & 0x01u;

    size_t dx = pos.x - rx;
    size_t dy = pos.y - ry;
    size_t dz = pos.z - rz;

    // Reminder: dy & 1 here flips between even and odd so it only caches the current 'deck' and the
    // one previous to it.

    const auto cell = cache[dy & 1u][(dx * chunkSize) + dz];
    return cell;
  }

  void setReusableIndex(glm::ivec3 pos, int8_t reuseIndex, uint16_t p) {
    cache[pos.y & 1u][pos.x * chunkSize + pos.z].vertices[reuseIndex] = p;
  }

private:
  std::array<std::vector<CellCacheEntry>, 2> cache;
  size_t chunkSize;
};

}
