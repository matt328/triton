#pragma once

namespace tr {

enum class BufferAlias : uint8_t {
  IndirectCommand = 0,
  IndirectCommandCount,
  ObjectData,
  ObjectPositions,
  ObjectRotations,
  ObjectScales,
  GeometryRegion,
  IndexData,
  VertexPositions,
  VertexNormal,
  VertexTexCoord,
  VertexColor,
  Count
};

enum class ImageAlias : uint8_t {
  GeometryColorImage = 0,
  SwapchainImage,
  DepthImage,
  Count
};

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
    case BufferAlias::IndexData:
      return "IndexData";
    case BufferAlias::VertexPositions:
      return "VertexPositions";
    case BufferAlias::VertexNormal:
      return "VertexNormal";
    case BufferAlias::VertexTexCoord:
      return "VertexTexCoord";
    case BufferAlias::VertexColor:
      return "VertexColor";
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
struct std::formatter<tr::BufferAlias> {
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
  constexpr auto parse(format_parse_context& ctx) {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto format(tr::ImageAlias alias, FormatContext& ctx) {
    return format_to(ctx.out(), "{}", to_string(alias));
  }
};
