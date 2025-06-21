#pragma once

#include "r3/graph/ResourceAliases.hpp"
#include "r3/graph/barriers/AccessMode.hpp"

namespace tr {
struct BufferBarrierPrecursor {
  BufferAliasVariant alias;
  AccessMode accessMode;
  vk::AccessFlags2 accessFlags;
  vk::PipelineStageFlags2 stageFlags;
};
}

template <>
struct std::formatter<tr::BufferBarrierPrecursor> {
  // NOLINTNEXTLINE
  constexpr auto parse(std::format_parse_context& ctx) {
    return ctx.begin();
  }

  // NOLINTNEXTLINE
  auto format(const tr::BufferBarrierPrecursor& bbp, std::format_context& ctx) const {
    return std::format_to(
        ctx.out(),
        "BufferBarrierPrecursor {{ alias={}, accessMode={}, accessFlags={}, stageFlags={} }}",
        bbp.alias,
        bbp.accessMode,
        vk::to_string(bbp.accessFlags),
        vk::to_string(bbp.stageFlags));
  }
};
