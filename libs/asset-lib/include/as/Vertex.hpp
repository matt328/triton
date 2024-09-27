#pragma once

#include "GlmCereal.hpp" // This is used during cerealization

namespace tr::as {
   struct Vertex {
      glm::vec3 pos{};
      glm::vec3 normal{};
      glm::vec2 uv{};
      glm::vec4 color{};
      glm::u8vec4 joint0{};
      glm::vec4 weight0{};
      glm::vec4 tangent{};

      auto operator==(const Vertex& other) const -> bool {
         return pos == other.pos && normal == other.normal && uv == other.uv &&
                color == other.color && joint0 == other.joint0 && weight0 == other.weight0 &&
                tangent == other.tangent;
      }

      template <class Archive>
      void serialize(Archive& archive) {
         archive(pos, normal, uv, color, joint0, weight0, tangent);
      }
   };
}