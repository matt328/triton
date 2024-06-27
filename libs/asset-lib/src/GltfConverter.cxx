#include "GltfConverter.hpp"
#include "GltfHelpers.hpp"

namespace al::gltf {
   Converter::~Converter() noexcept {
   }

   auto Converter::convert(const std::filesystem::path& gltf) -> ModelData {

      return ModelData{};
   }
}