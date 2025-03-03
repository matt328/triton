#pragma once

#include "GlmHashes.hpp"

namespace as {

struct TerrainVertex {
  glm::vec3 position{};
  glm::vec2 texCoord{};

  auto operator==(const TerrainVertex& other) const -> bool {
    return position == other.position && texCoord == other.texCoord;
  }

  template <class Archive>
  void serialize(Archive& archive) {
    archive(position, texCoord);
  }
};

}

template <>
struct fmt::formatter<as::TerrainVertex> {
  constexpr auto parse(fmt::format_parse_context& ctx) {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto format(const as::TerrainVertex& vert, FormatContext& ctx) {
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
struct hash<as::TerrainVertex> {
  auto operator()(const as::TerrainVertex& vtx) const -> std::size_t {
    std::size_t seed = 0;
    hash_combine(seed, std::hash<glm::vec3>{}(vtx.position));
    hash_combine(seed, std::hash<glm::vec2>{}(vtx.texCoord));
    return seed;
  }
};
}

template <typename T>
inline auto terrainVertexListHash(const T& vertices) -> std::size_t {
  std::size_t seed = 0;
  for (const auto& vertex : vertices) {
    hash_combine(seed, std::hash<as::TerrainVertex>{}(vertex));
  }
  return seed;
}
