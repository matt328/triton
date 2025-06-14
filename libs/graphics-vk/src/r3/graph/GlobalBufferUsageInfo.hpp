#pragma once

#include "r3/graph/ResourceAliases.hpp"

namespace tr {

struct GlobalBufferUsageInfo {
  GlobalBufferAlias alias;
  vk::AccessFlags2 accessFlags;
  vk::PipelineStageFlags2 stageFlags;
  vk::DeviceSize offset = 0;
  vk::DeviceSize size = VK_WHOLE_SIZE;

  auto operator==(const GlobalBufferUsageInfo& other) const -> bool {
    return alias == other.alias && accessFlags == other.accessFlags &&
           stageFlags == other.stageFlags && offset == other.offset && size == other.size;
  }
};

struct GlobalBufferUsageInfoHash {
  auto operator()(const GlobalBufferUsageInfo& b) const -> std::size_t {
    std::size_t h1 = std::hash<GlobalBufferAlias>()(b.alias);
    std::size_t h2 = std::hash<uint64_t>()(static_cast<uint64_t>(b.accessFlags));
    std::size_t h3 = std::hash<uint64_t>()(static_cast<uint64_t>(b.stageFlags));
    std::size_t h4 = std::hash<vk::DeviceSize>()(b.offset);
    std::size_t h5 = std::hash<vk::DeviceSize>()(b.size);
    return (((((h1 ^ (h2 << 1)) ^ (h3 << 1)) ^ (h4 << 1)) ^ (h5 << 1)));
  }
};

using GlobalBufferUsageInfoSet =
    std::unordered_set<GlobalBufferUsageInfo, GlobalBufferUsageInfoHash>;

}

template <>
struct std::formatter<tr::GlobalBufferUsageInfo> {
  constexpr auto parse(format_parse_context& ctx) {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto format(const tr::GlobalBufferUsageInfo& info, FormatContext& ctx) {
    return format_to(ctx.out(),
                     "{{ alias: {}, access: {}, stage: {}, offset: {}, size: {} }}",
                     to_string(info.alias),
                     vk::to_string(info.accessFlags),
                     vk::to_string(info.stageFlags),
                     info.offset,
                     info.size == VK_WHOLE_SIZE ? "VK_WHOLE_SIZE" : std::to_string(info.size));
  }
};
