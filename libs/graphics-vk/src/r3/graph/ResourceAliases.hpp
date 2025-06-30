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

inline auto to_string(GlobalBufferAlias alias) -> std::string {
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

inline auto to_string(BufferAlias alias) -> std::string {
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

using BufferAliasVariant = std::variant<BufferAlias, GlobalBufferAlias>;

// to_string
struct BufferAliasToStringVisitor {
   auto operator()(GlobalBufferAlias a) const -> std::string {
    return to_string(a);
  }
   auto operator()(BufferAlias a) const -> std::string {
    return to_string(a);
  }
};

constexpr auto to_string(const BufferAliasVariant& alias) -> std::string {
  return std::visit(BufferAliasToStringVisitor{}, alias);
}

}

// GlobalBufferAlias formatter
template <>
struct std::formatter<tr::GlobalBufferAlias> {
  constexpr auto parse(std::format_parse_context& ctx) {
    return ctx.begin();
  }
  auto format(tr::GlobalBufferAlias alias, std::format_context& ctx) const {
    return std::format_to(ctx.out(), "{}", to_string(alias));
  }
};

// BufferAlias formatter
template <>
struct std::formatter<tr::BufferAlias> {
  constexpr auto parse(std::format_parse_context& ctx) {
    return ctx.begin();
  }
  auto format(tr::BufferAlias alias, std::format_context& ctx) const {
    return std::format_to(ctx.out(), "{}", to_string(alias));
  }
};

// Generic variant formatter
template <typename... Ts>
struct std::formatter<std::variant<Ts...>> {
  constexpr auto parse(std::format_parse_context& ctx) {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto format(const std::variant<Ts...>& var, FormatContext& ctx) const {
    return std::visit(
        [&](const auto& value) {
          return std::formatter<std::decay_t<decltype(value)>>{}.format(value, ctx);
        },
        var);
  }
};

// Hash so it can be a key in a hashmap
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
