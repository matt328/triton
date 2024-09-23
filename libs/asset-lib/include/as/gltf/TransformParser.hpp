#pragma once

#include "as/ConverterComponents.hpp"

namespace tinygltf {
   class Node;
} // namespace tinygltf
namespace tr::as::gltf {

   class TransformParser final : public as::TransformParser {
    public:
      TransformParser() = default;
      ~TransformParser() noexcept override;

      TransformParser(const TransformParser&) = delete;
      auto operator=(const TransformParser&) -> TransformParser& = delete;

      TransformParser(TransformParser&&) = delete;
      auto operator=(TransformParser&&) -> TransformParser& = delete;

      [[nodiscard]] auto execute(const tinygltf::Node& node) const -> glm::mat4 override;
   };

} // namespace tr::as::gltf
