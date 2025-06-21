#pragma once

#include "r3/graph/ImageAlias.hpp"
#include "r3/graph/barriers/AccessMode.hpp"

namespace tr {

struct ImageBarrierPrecursor {
  ImageAlias alias;
  AccessMode accessMode;
  vk::AccessFlags2 accessFlags;
  vk::PipelineStageFlags2 stageFlags;
  vk::ImageLayout layout;
  vk::ImageAspectFlags aspectFlags;
};

}

template <>
struct std::formatter<tr::ImageBarrierPrecursor> {
  // NOLINTNEXTLINE
  constexpr auto parse(std::format_parse_context& ctx) {
    return ctx.begin();
  }

  // NOLINTNEXTLINE
  auto format(const tr::ImageBarrierPrecursor& ibp, std::format_context& ctx) const {
    return std::format_to(
        ctx.out(),
        "ImageBarrierPrecursor {{ alias={}, accessMode={}, accessFlags={}, stageFlags={}, "
        "layout={}, aspectFlags={} }}",
        ibp.alias,
        ibp.accessMode,
        vk::to_string(ibp.accessFlags),
        vk::to_string(ibp.stageFlags),
        vk::to_string(ibp.layout),
        vk::to_string(ibp.aspectFlags));
  }
};
