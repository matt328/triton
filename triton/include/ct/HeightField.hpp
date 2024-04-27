#pragma once

namespace tr::ct {
   class HeightField {
    public:
      HeightField(const int size);
      ~HeightField();

      HeightField(const HeightField&) = default;
      HeightField& operator=(const HeightField&) = default;

      HeightField(HeightField&&) = delete;
      HeightField& operator=(HeightField&&) = delete;

      [[nodiscard]] auto valueAt(uint32_t x, uint32_t y) const {
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