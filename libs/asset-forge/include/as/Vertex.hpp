#pragma once

#include "GlmHashes.hpp"

namespace as {
struct Vertex {
  glm::vec3 pos{};
  glm::vec3 normal{};
  glm::vec2 uv{};
  glm::vec4 color{};
  glm::u8vec4 joint0{};
  glm::vec4 weight0{};
  glm::vec4 tangent{};

  auto operator==(const Vertex& other) const -> bool {
    return pos == other.pos && normal == other.normal && uv == other.uv && color == other.color &&
           joint0 == other.joint0 && weight0 == other.weight0 && tangent == other.tangent;
  }

  template <class Archive>
  void serialize(Archive& archive) {
    archive(pos, normal, uv, color, joint0, weight0, tangent);
  }
};
}

template <>
struct std::formatter<as::Vertex> {
  constexpr auto parse(std::format_parse_context& ctx) {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto format(const as::Vertex& vert, FormatContext& ctx) const {
    return std::format_to(ctx.out(), "(pos: {}, {}, {})", vert.pos.x, vert.pos.y, vert.pos.z);
  }
};

namespace std {

template <>
struct hash<as::Vertex> {
  auto operator()(const as::Vertex& vtx) const -> std::size_t {
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
    hash_combine(seed, std::hash<as::Vertex>{}(vertex));
  }
  return seed;
}
