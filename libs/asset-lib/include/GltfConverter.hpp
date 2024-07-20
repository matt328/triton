#pragma once

#include "Model.hpp"

namespace tr::as::gltf {
   class Converter {
    public:
      Converter() = default;
      ~Converter() noexcept;

      Converter(const Converter&) = delete;
      Converter& operator=(const Converter&) = delete;

      Converter(Converter&&) = delete;
      Converter& operator=(Converter&&) = delete;

      /// Reads a gltf file and a skeleton in order to build the runtime structure representing an
      /// animated model.  The skeleton is only referenced in order to map the joints in the model
      /// to those in the skeleton.
      auto convert(const std::filesystem::path& gltf, const std::filesystem::path& skeleton)
          -> Model;
   };
}