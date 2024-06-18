#include "Converter.hpp"

namespace al {
   Converter::Converter() noexcept {
   }

   Converter::~Converter() {
   }

   auto Converter::convertFbx(std::filesystem::path& filePath) -> ModelData {
      // Extract a RawSkeleton
      // build a runtime skeleton with skeleton builder
      // do the same for animation
      // write the skeleton and animation to file

      // Extract the model data from the fbx file.
   }
}
