#pragma once

inline void hash_combine(std::size_t& seed, std::size_t hash) {
   seed ^= hash + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

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

template <>
struct fmt::formatter<tr::as::Vertex> {
   constexpr auto parse(fmt::format_parse_context& ctx) {
      return ctx.begin();
   }

   template <typename FormatContext>
   auto format(const tr::as::Vertex& vert, FormatContext& ctx) {
      return fmt::format_to(ctx.out(), "(pos: {}, {}, {})", vert.pos.x, vert.pos.y, vert.pos.z);
   }
};

namespace std {
   template <>
   struct hash<glm::vec2> {
      auto operator()(const glm::vec2& vec) const -> std::size_t {
         std::size_t seed = 0;
         std::hash<float> hasher;
         hash_combine(seed, hasher(vec.x));
         hash_combine(seed, hasher(vec.y));
         return seed;
      }
   };

   template <>
   struct hash<glm::vec3> {
      auto operator()(const glm::vec3& vec) const -> std::size_t {
         std::size_t seed = 0;
         std::hash<float> hasher;
         hash_combine(seed, hasher(vec.x));
         hash_combine(seed, hasher(vec.y));
         hash_combine(seed, hasher(vec.z));
         return seed;
      }
   };

   template <>
   struct hash<glm::vec4> {
      auto operator()(const glm::vec4& vec) const -> std::size_t {
         std::size_t seed = 0;
         std::hash<float> hasher;
         hash_combine(seed, hasher(vec.x));
         hash_combine(seed, hasher(vec.y));
         hash_combine(seed, hasher(vec.z));
         hash_combine(seed, hasher(vec.w));
         return seed;
      }
   };

   template <>
   struct hash<glm::u8vec4> {
      auto operator()(const glm::u8vec4& vec) const -> std::size_t {
         std::size_t seed = 0;
         std::hash<uint8_t> hasher;
         hash_combine(seed, hasher(vec.r));
         hash_combine(seed, hasher(vec.g));
         hash_combine(seed, hasher(vec.b));
         hash_combine(seed, hasher(vec.a));
         return seed;
      }
   };

   template <>
   struct hash<tr::as::Vertex> {
      auto operator()(const tr::as::Vertex& vtx) const -> std::size_t {
         std::size_t seed = 0;
         hash_combine(seed, std::hash<glm::vec3>{}(vtx.pos));
         hash_combine(seed, std::hash<glm::vec3>{}(vtx.normal));
         hash_combine(seed, std::hash<glm::vec2>{}(vtx.uv));
         hash_combine(seed, std::hash<glm::vec4>{}(vtx.color));
         hash_combine(seed, std::hash<glm::u8vec4>{}(vtx.joint0));
         hash_combine(seed, std::hash<glm::vec4>{}(vtx.weight0));
         hash_combine(seed, std::hash<glm::vec4>{}(vtx.tangent));

         return seed;
      }
   };
}

template <typename T>
inline auto vertexListHash(const T& vertices) -> std::size_t {
   std::size_t seed = 0;
   for (const auto& vertex : vertices) {
      hash_combine(seed, std::hash<tr::as::Vertex>{}(vertex));
   }
   return seed;
}