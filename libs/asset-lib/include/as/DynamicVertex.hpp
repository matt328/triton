#pragma once

#include "GlmHashes.hpp"
#include "GlmCereal.hpp"

namespace as {

struct DynamicVertex {
  glm::vec3 position{};
  glm::vec2 texCoord{};
  glm::u8vec4 joint0{};
  glm::vec4 weight0{};

  auto operator==(const DynamicVertex& other) const -> bool {
    return position == other.position && texCoord == other.texCoord && joint0 == other.joint0 &&
           weight0 == other.weight0;
  }

  template <class Archive>
  void serialize(Archive& archive) {
    archive(position, texCoord, joint0, weight0);
  }
};

}

template <>
struct fmt::formatter<as::DynamicVertex> {
  constexpr auto parse(fmt::format_parse_context& ctx) {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto format(const as::DynamicVertex& vert, FormatContext& ctx) {
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
struct hash<as::DynamicVertex> {
  auto operator()(const as::DynamicVertex& vtx) const -> std::size_t {
    std::size_t seed = 0;
    hash_combine(seed, std::hash<glm::vec3>{}(vtx.position));
    hash_combine(seed, std::hash<glm::vec2>{}(vtx.texCoord));
    hash_combine(seed, std::hash<glm::u8vec4>{}(vtx.joint0));
    hash_combine(seed, std::hash<glm::vec4>{}(vtx.weight0));
    return seed;
  }
};
}

template <typename T>
inline auto skinnedVertexListHash(const T& vertices) -> std::size_t {
  std::size_t seed = 0;
  for (const auto& vertex : vertices) {
    hash_combine(seed, std::hash<as::DynamicVertex>{}(vertex));
  }
  return seed;
}
