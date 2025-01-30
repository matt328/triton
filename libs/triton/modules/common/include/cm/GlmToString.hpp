#pragma once

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
