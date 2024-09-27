#include "sdf/DistanceField.hpp"
#include "sdf/FastNoiseLite.h"

namespace sdf {

   using NoiseFn = std::function<float(float xCoord, float yCoord, float zCoord)>;

   DistanceField::DistanceField(const NoiseParams& noiseParams) {
      perlinNoise = std::make_unique<FastNoiseLite>();
      perlinNoise->SetFrequency(noiseParams.frequency);
      perlinNoise->SetNoiseType(FastNoiseLite::NoiseType_Perlin);
      perlinNoise->SetFractalLacunarity(noiseParams.lacunarity);
   }

   DistanceField::~DistanceField() = default;

   auto DistanceField::getNoiseValue(float xCoord, float yCoord, float zCoord) const -> float {
      auto value = perlinNoise->GetNoise(xCoord / factor, yCoord / factor, zCoord / factor);
      return static_cast<float>(value);
   }

   auto DistanceField::getSimplexValue(float xCoord, float yCoord, float zCoord, float div) const
       -> int8_t {
      auto value = perlinNoise->GetNoise(xCoord / div, yCoord / div, zCoord / div) * 128.0;
      return static_cast<int8_t>(value);
   }

   auto DistanceField::getValue(float xCoord, float yCoord, float zCoord) const -> float {
      auto noiseValue = getNoiseValue(xCoord, yCoord, zCoord);
      auto [xc, yc, zc] = computeGradient(xCoord, yCoord, zCoord);

      auto gradientMagnitude = std::sqrt(xc * xc + yc * yc + zc * zc);

      if (gradientMagnitude < 0.0001F) {
         return noiseValue;
      }

      auto distanceToIsoSurface = noiseValue / gradientMagnitude;

      return distanceToIsoSurface;
   }

   auto DistanceField::computeGradient(float xCoord, float yCoord, float zCoord) const
       -> std::tuple<float, float, float> {
      constexpr float epsilon = 0.001F;
      return {(getNoiseValue(xCoord + epsilon, 0, 0) - getNoiseValue(xCoord - epsilon, 0, 0)) /
                  (2 * epsilon),
              (getNoiseValue(0, yCoord + epsilon, 0) - getNoiseValue(0, yCoord - epsilon, 0)) /
                  (2 * epsilon),
              (getNoiseValue(0, 0, zCoord + epsilon) - getNoiseValue(0, 0, zCoord - epsilon)) /
                  (2 * epsilon)};
   }

} // namespace sdf
