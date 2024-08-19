#pragma once

#include "as/ConverterComponents.hpp"

namespace tr::as {
   struct Model;
}

namespace tinygltf {
   class Model;
}

namespace tr::as::gltf {

   class TextureExtractor final : public as::TextureExtractor {
    public:
      TextureExtractor() = default;
      ~TextureExtractor() noexcept override;

      TextureExtractor(const TextureExtractor&) = delete;
      TextureExtractor& operator=(const TextureExtractor&) = delete;

      TextureExtractor(TextureExtractor&&) = delete;
      TextureExtractor& operator=(TextureExtractor&&) = delete;

      void execute(const tinygltf::Model& model, int textureIndex, Model& tritonModel) override;
   };

}
