#include "ct/HeightField.hpp"

#include <noise/noise.h>
#include <noise/noiseutils.h>

namespace tr::ct {

   using namespace noise;
   constexpr double LowerX = 2.0;
   constexpr double UpperX = 6.0;
   constexpr double LowerZ = 1.0;
   constexpr double UpperZ = 5.0;
   constexpr float MaxHeight = 20;

   HeightField::HeightField(const int size) : width{size} {
      module::Perlin myModule;
      utils::NoiseMap heightMap;
      utils::NoiseMapBuilderPlane heightMapBuilder;
      heightMapBuilder.SetSourceModule(myModule);
      heightMapBuilder.SetDestNoiseMap(heightMap);
      heightMapBuilder.SetDestSize(size, size);
      heightMapBuilder.SetBounds(LowerX, UpperX, LowerZ, UpperZ);
      heightMapBuilder.Build();

      for (int x = 0; x < size; x++) {
         for (int y = 0; y < size; y++) {
            auto value = heightMap.GetValue(x, y);

            value = value + 1.0f;
            value = value * MaxHeight / 2;
            data.push_back(value);
         }
      }
   }

   HeightField::~HeightField() {
   }
}
