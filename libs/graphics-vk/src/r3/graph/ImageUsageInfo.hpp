#pragma once

#include "r3/graph/ResourceAliases.hpp"

namespace tr {

struct ImageUsageInfo {
  ImageAlias alias;
  vk::AccessFlags2 accessFlags;
  vk::PipelineStageFlags2 stageFlags;
  vk::ImageAspectFlags aspectFlags;
  vk::ImageLayout layout;
  vk::ClearValue clearValue;
};

// Non-member equality operator
inline auto operator==(const ImageUsageInfo& lhs, const ImageUsageInfo& rhs) -> bool {
  return lhs.alias == rhs.alias && lhs.accessFlags == rhs.accessFlags &&
         lhs.stageFlags == rhs.stageFlags && lhs.aspectFlags == rhs.aspectFlags &&
         lhs.layout == rhs.layout &&
         std::memcmp(&lhs.clearValue, &rhs.clearValue, sizeof(vk::ClearValue)) == 0;
}

}

// std::hash specialization for ImageUsageInfo
namespace std {
template <>
struct hash<tr::ImageUsageInfo> {
  auto operator()(const tr::ImageUsageInfo& info) const noexcept -> size_t {
    size_t h = std::hash<int>{}(static_cast<int>(info.alias)); // or a suitable hash of alias
    h ^= std::hash<uint32_t>{}(static_cast<uint64_t>(info.accessFlags)) + 0x9e3779b9 + (h << 6) +
         (h >> 2);
    h ^= std::hash<uint32_t>{}(static_cast<uint64_t>(info.stageFlags)) + 0x9e3779b9 + (h << 6) +
         (h >> 2);
    h ^= std::hash<uint32_t>{}(static_cast<uint32_t>(info.aspectFlags)) + 0x9e3779b9 + (h << 6) +
         (h >> 2);
    h ^= std::hash<int>{}(static_cast<int>(info.layout)) + 0x9e3779b9 + (h << 6) + (h >> 2);

    // For vk::ClearValue, do a simple hash of the raw bytes
    const uint8_t* bytes = reinterpret_cast<const uint8_t*>(&info.clearValue);
    size_t clearHash = 0;
    for (size_t i = 0; i < sizeof(vk::ClearValue); ++i) {
      clearHash = clearHash * 31 + bytes[i];
    }
    h ^= clearHash + 0x9e3779b9 + (h << 6) + (h >> 2);

    return h;
  }
};
}

template <>
struct std::formatter<tr::ImageUsageInfo> {
  constexpr auto parse(format_parse_context& ctx) {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto format(const tr::ImageUsageInfo& info, FormatContext& ctx) {
    std::string clearStr = "[ClearValue]";

    return std::format_to(
        ctx.out(),
        "ImageUsageInfo {{ alias={}, access={}, stage={}, aspect={}, layout={}, clear={} }}",
        static_cast<uint8_t>(info.alias),
        vk::to_string(info.accessFlags),
        vk::to_string(info.stageFlags),
        vk::to_string(info.aspectFlags),
        vk::to_string(info.layout),
        clearStr);
  }
};
