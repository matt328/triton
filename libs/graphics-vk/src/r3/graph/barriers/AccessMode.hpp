#pragma once

namespace tr {

enum class AccessMode : uint8_t {
  Read = 0,
  Write,
};

}

constexpr auto to_string(tr::AccessMode mode) -> std::string_view {
  switch (mode) {
    case tr::AccessMode::Read:
      return "Read";
    case tr::AccessMode::Write:
      return "Write";
    default:
      return "Unknown";
  }
}

template <>
struct std::formatter<tr::AccessMode> {
  // NOLINTNEXTLINE
  constexpr auto parse(std::format_parse_context& ctx) {
    return ctx.begin();
  }

  // NOLINTNEXTLINE
  auto format(tr::AccessMode mode, std::format_context& ctx) const {
    return std::format_to(ctx.out(), "{}", ::to_string(mode));
  }
};
