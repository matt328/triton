#pragma once

#include "ModelData.hpp"

namespace al {
   class Converter {
    public:
      Converter() noexcept;
      ~Converter() noexcept;

      auto convertFbx(std::filesystem::path& filePath) -> ModelData;
   };
}