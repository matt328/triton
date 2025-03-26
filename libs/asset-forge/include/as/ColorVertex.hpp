#pragma once

#include "GlmHashes.hpp"

namespace as {

struct ColorVertex {
  glm::vec3 position{};
  glm::vec3 color{1.f, 1.f, 1.f};

  auto operator==(const ColorVertex& other) const -> bool {
    return position == other.position && color == other.color;
  }

  template <class Archive>
  void serialize(Archive& archive) {
    archive(position, color);
  }
};

}

template <>
struct fmt::formatter<as::ColorVertex> {
  constexpr auto parse(fmt::format_parse_context& ctx) {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto format(const as::ColorVertex& vert, FormatContext& ctx) {
    return fmt::format_to(ctx.out(),
                          "position: ({}, {}, {}), color: ({}, {}, {})",
                          vert.position.x,
                          vert.position.y,
                          vert.position.z,
                          vert.color.x,
                          vert.color.y,
                          vert.color.z);
  }
};

namespace std {

template <>
struct hash<as::ColorVertex> {
  auto operator()(const as::ColorVertex& vtx) const -> std::size_t {
    std::size_t seed = 0;
    hash_combine(seed, std::hash<glm::vec3>{}(vtx.position));
    hash_combine(seed, std::hash<glm::vec3>{}(vtx.color));
    return seed;
  }
};
}

template <typename T>
inline auto colorVertexListHash(const T& vertices) -> std::size_t {
  std::size_t seed = 0;
  for (const auto& vertex : vertices) {
    hash_combine(seed, std::hash<as::ColorVertex>{}(vertex));
  }
  return seed;
}
