#pragma once

#include "GlmHashes.hpp"

namespace as {

struct StaticVertex {
  glm::vec3 position{};
  glm::vec2 texCoord{};

  auto operator==(const StaticVertex& other) const -> bool {
    return position == other.position && texCoord == other.texCoord;
  }

  template <class Archive>
  void serialize(Archive& archive) {
    archive(position, texCoord);
  }
};

}

template <>
struct fmt::formatter<as::StaticVertex> {
  constexpr auto parse(fmt::format_parse_context& ctx) {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto format(const as::StaticVertex& vert, FormatContext& ctx) {
    return fmt::format_to(ctx.out(),
                          "position: ({}, {}, {}), texCoord: ({}, {})",
                          vert.position.x,
                          vert.position.y,
                          vert.position.z,
                          vert.texCoord.x,
                          vert.texCoord.y);
  }
};

namespace std {

template <>
struct hash<as::StaticVertex> {
  auto operator()(const as::StaticVertex& vtx) const -> std::size_t {
    std::size_t seed = 0;
    hash_combine(seed, std::hash<glm::vec3>{}(vtx.position));
    hash_combine(seed, std::hash<glm::vec2>{}(vtx.texCoord));
    return seed;
  }
};
}

template <typename T>
inline auto staticVertexListHash(const T& vertices) -> std::size_t {
  std::size_t seed = 0;
  for (const auto& vertex : vertices) {
    hash_combine(seed, std::hash<as::StaticVertex>{}(vertex));
  }
  return seed;
}
