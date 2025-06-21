#pragma once

namespace tr {
enum class ImageAlias : uint8_t {
  GeometryColorImage = 0,
  SwapchainImage,
  DepthImage,
  GuiColorImage,
  Count
};

// ImageAlias to_string
inline auto to_string(ImageAlias alias) -> std::string {
  switch (alias) {
    case ImageAlias::GeometryColorImage:
      return "GeometryColorImage";
    case ImageAlias::SwapchainImage:
      return "SwapchainImage";
    case ImageAlias::DepthImage:
      return "DepthImage";
    case ImageAlias::GuiColorImage:
      return "GuiColorImage";
    case ImageAlias::Count:
      return "Count";
    default:
      return "UnknownImageAlias";
  }
}

}

// ImageAlias Formatter
template <>
struct std::formatter<tr::ImageAlias> {
  // NOLINTNEXTLINE
  constexpr auto parse(format_parse_context& ctx) {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto format(tr::ImageAlias alias, FormatContext& ctx) const {
    return std::format_to(ctx.out(), "{}", to_string(alias));
  }
};

// ImageAlias Hash
namespace std {
template <>
struct hash<tr::ImageAlias> {
  auto operator()(const tr::ImageAlias& alias) const noexcept -> std::size_t {
    using Underlying = std::underlying_type_t<tr::ImageAlias>;
    return std::hash<Underlying>{}(static_cast<Underlying>(alias));
  }
};
}
