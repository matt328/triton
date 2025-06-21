#pragma once

#include "r3/ComponentIds.hpp"
#include "r3/graph/barriers/BufferBarrierPrecursor.hpp"
#include "r3/graph/barriers/ImageBarrierPrecursor.hpp"

namespace tr {

struct BarrierPrecursorPlan {
  std::unordered_map<PassId, std::vector<ImageBarrierPrecursor>> imagePrecursors;
  std::unordered_map<PassId, std::vector<BufferBarrierPrecursor>> bufferPrecursors;
};

}

template <>
struct std::formatter<tr::BarrierPrecursorPlan> {
  // NOLINTNEXTLINE
  constexpr auto parse(std::format_parse_context& ctx) {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto format(const tr::BarrierPrecursorPlan& plan, FormatContext& ctx) const {
    auto out = ctx.out();
    out = std::format_to(out, "BarrierPrecursorPlan {{\n");

    out = std::format_to(out, "  ImagePrecursors:\n");
    for (const auto& [passId, precursors] : plan.imagePrecursors) {
      out = std::format_to(out, "    [{}]:\n", passId);
      for (const auto& precursor : precursors) {
        out = std::format_to(out, "      {}\n", precursor);
      }
    }

    out = std::format_to(out, "  BufferPrecursors:\n");
    for (const auto& [passId, precursors] : plan.bufferPrecursors) {
      out = std::format_to(out, "    [{}]:\n", passId);
      for (const auto& precursor : precursors) {
        out = std::format_to(out, "      {}\n", precursor);
      }
    }

    return std::format_to(out, "}}\n");
  }
};
