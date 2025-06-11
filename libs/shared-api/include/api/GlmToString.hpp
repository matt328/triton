#pragma once

template <>
struct std::formatter<glm::vec3> {
  constexpr auto parse(std::format_parse_context& ctx) {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto format(const glm::vec3& vec, FormatContext& ctx) const {
    return std::format_to(ctx.out(), "[{}, {}, {}]", vec.x, vec.y, vec.z);
  }
};

template <>
struct std::formatter<glm::ivec3> {
  constexpr auto parse(std::format_parse_context& ctx) {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto format(const glm::ivec3& vec, FormatContext& ctx) const {
    return std::format_to(ctx.out(), "[{}, {}, {}]", vec.x, vec.y, vec.z);
  }
};
