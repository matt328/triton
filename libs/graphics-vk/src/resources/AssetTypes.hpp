#pragma once

#include "api/fx/ResourceEvents.hpp"

namespace tr {

struct StaticModelTask {
  uint64_t batchId;
  size_t id{0};
  std::string filename;
  std::string entityName;
};

struct DynamicModelTask {
  uint64_t batchId;
  size_t id{0};
  std::string filename;
  std::string entityName;
};

using AssetRequest = std::variant<StaticModelRequest, DynamicModelRequest>;

using AssetTask = std::variant<StaticModelTask, DynamicModelTask>;

template <typename>
inline constexpr bool always_false = false;

}
