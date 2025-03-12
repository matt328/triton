#pragma once

#include <format>
#include "glm/glm.hpp"

template <>
struct fmt::formatter<glm::vec3> {
  constexpr auto parse(fmt::format_parse_context& ctx) {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto format(const glm::vec3& vec, FormatContext& ctx) {
    return fmt::format_to(ctx.out(), "[{}, {}, {}]", vec.x, vec.y, vec.z);
  }
};

template <>
struct fmt::formatter<glm::ivec3> {
  constexpr auto parse(fmt::format_parse_context& ctx) {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto format(const glm::ivec3& vec, FormatContext& ctx) {
    return fmt::format_to(ctx.out(), "[{}, {}, {}]", vec.x, vec.y, vec.z);
  }
};
