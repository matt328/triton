#pragma once

namespace tr {

struct RenderConfigHandle {
  uint64_t id;

  auto operator==(const RenderConfigHandle&) const -> bool = default;
};

}

namespace std {
template <>
struct hash<tr::RenderConfigHandle> {
  auto operator()(const tr::RenderConfigHandle& handle) const noexcept -> size_t {
    return std::hash<uint64_t>{}(handle.id);
  }
};
}
