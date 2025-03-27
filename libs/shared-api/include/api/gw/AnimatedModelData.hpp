#pragma once

namespace tr {

struct AnimatedModelData {
  std::string modelFilename;
  std::string skeletonFilename;
  std::string animationFilename;
  std::optional<std::string> entityName = std::nullopt;
};

}

// Specialize fmt::formatter for AnimatedModelData
template <>
struct fmt::formatter<tr::AnimatedModelData> {
  constexpr auto parse(fmt::format_parse_context& ctx) -> decltype(ctx.begin()) {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto format(const tr::AnimatedModelData& data, FormatContext& ctx) -> decltype(ctx.out()) {
    return fmt::format_to(
        ctx.out(),
        "AnimatedModelData(modelFilename: '{}', skeletonFilename: '{}', animationFilename: '{}')",
        data.modelFilename,
        data.skeletonFilename,
        data.animationFilename);
  }
};
