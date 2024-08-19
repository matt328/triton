#pragma once
#include "as/ConverterComponents.hpp"

namespace tr::as::gltf {

   class GeometryExtractor final : public as::GeometryExtractor {
    public:
      GeometryExtractor() = default;
      ~GeometryExtractor() noexcept override;

      GeometryExtractor(const GeometryExtractor&) = delete;
      GeometryExtractor& operator=(const GeometryExtractor&) = delete;

      GeometryExtractor(GeometryExtractor&&) = delete;
      GeometryExtractor& operator=(GeometryExtractor&&) = delete;

      void execute(const tinygltf::Model& model,
                   const tinygltf::Primitive& primitive,
                   const glm::mat4& transform,
                   Model& tritonModel) override;
   };

}