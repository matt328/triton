#pragma once

#include "ModelData.hpp"

namespace al::gltf {
   class Converter {
    public:
      Converter() = default;
      ~Converter() noexcept;

      Converter(const Converter&) = delete;
      Converter& operator=(const Converter&) = delete;

      Converter(Converter&&) = delete;
      Converter& operator=(Converter&&) = delete;

      auto convert(const std::filesystem::path& gltf,
                   const std::filesystem::path& skeleton,
                   const std::filesystem::path& animation) -> ModelData;
   };
}