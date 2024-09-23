#include "HeightField.hpp"

#include <sdf/FastNoiseLite.h>

namespace tr::ct {

   HeightField::HeightField(const int size) : width{size} {

      auto fn = FastNoiseLite();

      for (int x = 0; x < size; x++) {
         for (int y = 0; y < size; y++) {
            auto value = fn.GetNoise(static_cast<float>(x), static_cast<float>(y));

            value = value + 1.0f;
            value = value * 512.f / 2;
            data.push_back(value);
         }
      }
   }

   HeightField::~HeightField() { // NOLINT(*-use-equals-default)
   }
}
