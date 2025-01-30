#pragma once

namespace tr::ct {
class HeightField {
public:
  explicit HeightField(int size);
  ~HeightField();

  HeightField(const HeightField&) = default;
  HeightField& operator=(const HeightField&) = default;

  HeightField(HeightField&&) = delete;
  HeightField& operator=(HeightField&&) = delete;

  [[nodiscard]] auto valueAt(const uint32_t x, const uint32_t y) const {
    return data[y * width + x];
  }

  [[nodiscard]] auto getWidth() const {
    return width;
  }

private:
  int width;
  std::vector<float> data;
};
}
