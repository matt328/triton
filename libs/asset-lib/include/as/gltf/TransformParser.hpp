#pragma once

#include "as/ConverterComponents.hpp"

namespace tinygltf {
   class Node;
}
namespace tr::as::gltf {

   class TransformParser final : public as::TransformParser {
    public:
      TransformParser() = default;
      ~TransformParser() noexcept override;

      TransformParser(const TransformParser&) = delete;
      TransformParser& operator=(const TransformParser&) = delete;

      TransformParser(TransformParser&&) = delete;
      TransformParser& operator=(TransformParser&&) = delete;

      [[nodiscard]] glm::mat4 execute(const tinygltf::Node& node) const override;
   };

}
