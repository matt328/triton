#pragma once

namespace tr {

enum class BufferAlias : uint8_t {
  IndirectCommand = 0,
  IndirectCommandCount,
  IndirectMetaData,
  ObjectData,
  ObjectPositions,
  ObjectRotations,
  ObjectScales,
  GeometryRegion,
  FrameData,
  ResourceTable,
  Count
};

enum class GlobalBufferAlias : uint8_t {
  Index = 0,
  Position,
  Normal,
  TexCoord,
  Color,
  Animation,
  Count
};

enum class ImageAlias : uint8_t {
  GeometryColorImage = 0,
  SwapchainImage,
  DepthImage,
  Count
};

using BufferAliasVariant = std::variant<BufferAlias, GlobalBufferAlias>;

constexpr auto to_string(const BufferAliasVariant& alias) -> std::string_view {
  return std::visit([](auto&& a) -> std::string_view { return to_string(a); }, alias);
}

constexpr auto to_string(GlobalBufferAlias alias) -> std::string_view {
  switch (alias) {
    case GlobalBufferAlias::Index:
      return "Index";
    case GlobalBufferAlias::Position:
      return "Position";
    case GlobalBufferAlias::Normal:
      return "Normal";
    case GlobalBufferAlias::TexCoord:
      return "TexCoord";
    case GlobalBufferAlias::Color:
      return "Color";
    case GlobalBufferAlias::Animation:
      return "Animation";
    case tr::GlobalBufferAlias::Count:
      return "Count";
  }
}

constexpr auto to_string(BufferAlias alias) -> std::string_view {
  switch (alias) {
    case BufferAlias::IndirectCommand:
      return "IndirectCommand";
    case BufferAlias::IndirectCommandCount:
      return "IndirectCommandCount";
    case BufferAlias::ObjectData:
      return "ObjectData";
    case BufferAlias::ObjectPositions:
      return "ObjectPositions";
    case BufferAlias::ObjectRotations:
      return "ObjectRotations";
    case BufferAlias::ObjectScales:
      return "ObjectScales";
    case BufferAlias::GeometryRegion:
      return "GeometryRegion";
    case BufferAlias::Count:
      return "Count";
    default:
      return "UnknownBufferAlias";
  }
}

constexpr auto to_string(ImageAlias alias) -> std::string_view {
  switch (alias) {
    case ImageAlias::GeometryColorImage:
      return "GeometryColorImage";
    case ImageAlias::SwapchainImage:
      return "SwapchainImage";
    case ImageAlias::DepthImage:
      return "DepthImage";
    case ImageAlias::Count:
      return "Count";
    default:
      return "UnknownImageAlias";
  }
}

}

template <>
struct std::formatter<tr::GlobalBufferAlias> {
  // NOLINTNEXTLINE
  constexpr auto parse(format_parse_context& ctx) {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto format(tr::GlobalBufferAlias alias, FormatContext& ctx) {
    return format_to(ctx.out(), "{}", to_string(alias));
  }
};

template <>
struct std::formatter<tr::BufferAlias> {
  // NOLINTNEXTLINE
  constexpr auto parse(format_parse_context& ctx) {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto format(tr::BufferAlias alias, FormatContext& ctx) {
    return format_to(ctx.out(), "{}", to_string(alias));
  }
};

template <>
struct std::formatter<tr::ImageAlias> {
  // NOLINTNEXTLINE
  constexpr auto parse(format_parse_context& ctx) {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto format(tr::ImageAlias alias, FormatContext& ctx) {
    return format_to(ctx.out(), "{}", to_string(alias));
  }
};

namespace std {
template <>
struct hash<tr::BufferAliasVariant> {
  auto operator()(const tr::BufferAliasVariant& alias) const -> std::size_t {
    return std::visit(
        [](auto&& a) -> std::size_t {
          using T = std::decay_t<decltype(a)>;
          return std::hash<T>{}(a) ^ (typeid(T).hash_code() << 1u);
        },
        alias);
  }
};
}
