#pragma once

#include "api/fx/Events.hpp"

namespace tr {

struct StaticModelTask {
  size_t id{0};
  std::string filename;
  std::string entityName;
};

using AssetRequest = std::variant<StaticModelRequest>;

using AssetTask = std::variant<StaticModelTask>;

template <typename>
inline constexpr bool always_false = false;

}
