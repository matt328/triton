#pragma once

namespace tr {

struct AnimatedModelData {
  std::string modelFilename;
  std::string skeletonFilename;
  std::string animationFilename;
  std::optional<std::string> entityName = std::nullopt;
};

}

template <>
struct std::formatter<tr::AnimatedModelData> {
  constexpr auto parse(std::format_parse_context& ctx) {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto format(const tr::AnimatedModelData& data, FormatContext& ctx) const {
    return std::format_to(
        ctx.out(),
        "AnimatedModelData(modelFilename: '{}', skeletonFilename: '{}', animationFilename: '{}')",
        data.modelFilename,
        data.skeletonFilename,
        data.animationFilename);
  }
};
