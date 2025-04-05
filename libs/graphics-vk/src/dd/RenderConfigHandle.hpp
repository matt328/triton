#pragma once

namespace tr {
struct RenderConfigHandle {
  uint64_t id;

  auto operator==(const RenderConfigHandle&) const -> bool = default;
};
}
