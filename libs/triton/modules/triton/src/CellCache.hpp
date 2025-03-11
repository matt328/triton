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

  auto getReusedIndex(glm::ivec3 pos, int8_t dir) -> CellCacheEntry {
    int rx = dir & 0x01;
    int rz = (dir >> 1) & 0x01;
    int ry = (dir >> 2) & 0x01;

    int dx = pos.x - rx;
    int dy = pos.y - ry;
    int dz = pos.z - rz;

    Log.debug("Returning CellCacheEntry at [{},{},{}]", dx, dy, dz);
    const auto cell = cache[dy & 1][dx * chunkSize + dz];
    return cell;
  }

  void setReusableIndex(glm::ivec3 pos, int8_t reuseIndex, uint16_t p) {
    cache[pos.y & 1][pos.x * chunkSize + pos.z].vertices[reuseIndex] = p;
  }

private:
  std::array<std::vector<CellCacheEntry>, 2> cache;
  size_t chunkSize;
};

}
