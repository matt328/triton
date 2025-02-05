#pragma once

namespace tr {

struct BufferRegion {
  size_t offset{};
  size_t size{};

  auto operator==(const BufferRegion& other) const -> bool {
    return offset == other.offset && size == other.size;
  }
};

struct BufferRegionComparator {
  auto operator()(const BufferRegion& a, const BufferRegion& b) const -> bool {
    if (a.size != b.size) {
      return a.size < b.size;
    }
    return a.offset < b.offset;
  }
};

}
