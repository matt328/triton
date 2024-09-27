#include "FastNoiseLite.h"

namespace sdf {

   struct NoiseParams {
      static constexpr float DefaultFrequency = 1.0;
      static constexpr float DefaultLacunarity = 2.0;
      static constexpr double DefaultPersistence = 0.5;
      static constexpr int DefaultOctaveCount = 6;

      float frequency{DefaultFrequency};
      float lacunarity{DefaultLacunarity};
      double persistence{DefaultPersistence};
      int octaveCount{DefaultOctaveCount};
   };

   class DistanceField {
    public:
      explicit DistanceField(const NoiseParams& noiseParams = NoiseParams{});
      virtual ~DistanceField();

      DistanceField(const DistanceField&) = delete;
      DistanceField(DistanceField&&) = delete;
      auto operator=(const DistanceField&) -> DistanceField& = delete;
      auto operator=(DistanceField&&) -> DistanceField& = delete;

      [[nodiscard]] auto getValue(float xCoord, float yCoord, float zCoord) const -> float;

      [[nodiscard]] auto getSimplexValue(float xCoord, float yCoord, float zCoord, float div) const
          -> int8_t;

    private:
      static constexpr double Frequency = 16.0;
      static constexpr double Amplitude = 1.5;
      static constexpr double Offset = 0.001;
      static constexpr auto factor = Frequency * Amplitude + Offset;

      std::unique_ptr<FastNoiseLite> perlinNoise;

      /// Computes the gradient of the noise function using finite differences
      [[nodiscard]] auto computeGradient(float xCoord, float yCoord, float zCoord) const
          -> std::tuple<float, float, float>;

      /// Noise generation function
      [[nodiscard]] auto getNoiseValue(float xCoord, float yCoord, float zCoord) const -> float;
   };
} // namespace sdf
