#pragma once

namespace tr {

struct ObjectBufferConfig {
  size_t stride;
  bool hasMaterialId = false;
  bool hasAnimationDataId = false;

  auto operator==(const ObjectBufferConfig& other) const -> bool {
    return stride == other.stride && hasMaterialId == other.hasMaterialId &&
           hasAnimationDataId == other.hasAnimationDataId;
  }
};

}

namespace std {
template <>
struct hash<tr::ObjectBufferConfig> {
  auto operator()(const tr::ObjectBufferConfig& config) const noexcept -> std::size_t {
    return config.stride |
           static_cast<std::size_t>(static_cast<uint8_t>(config.hasMaterialId) |
                                    (static_cast<uint8_t>(config.hasAnimationDataId) << 1u));
  }
};

}
