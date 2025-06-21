#pragma once

namespace tr {

enum class PassId : uint8_t {
  Culling = 0,
  Forward,
  Composition,
  PostProcessing,
  ImGui,
  Present,
};

enum class ContextId : uint8_t {
  Culling = 0,
  Cube,
  Composition,
  ImGui,
};

}

template <>
struct std::formatter<tr::PassId> {
  // NOLINTNEXTLINE
  constexpr auto parse(std::format_parse_context& ctx) {
    return ctx.begin();
  }

  // NOLINTNEXTLINE
  auto format(tr::PassId id, std::format_context& ctx) const {
    std::string_view name = "Unknown";
    switch (id) {
      case tr::PassId::Culling:
        name = "Culling";
        break;
      case tr::PassId::Forward:
        name = "Forward";
        break;
      case tr::PassId::Composition:
        name = "Composition";
        break;
      case tr::PassId::PostProcessing:
        name = "PostProcessing";
        break;
      case tr::PassId::ImGui:
        name = "ImGui";
        break;
      case tr::PassId::Present:
        name = "Present";
        break;
    }
    return std::format_to(ctx.out(), "{}", name);
  }
};
