#include "RenderConfigRegistry.hpp"

namespace tr {

auto RenderConfigRegistry::registerOrGet(const RenderConfig& target) -> RenderConfigHandle {
  for (const auto& [handle, config] : configs) {
    if (config == target) {
      return handle;
    }
  }

  const auto handle = RenderConfigHandle{.id = configKeygen.getKey()};
  configs.emplace(handle, target);
  return handle;
}

[[nodiscard]] auto RenderConfigRegistry::get(RenderConfigHandle handle) const
    -> const RenderConfig& {
  assert(configs.contains(handle) && "Who even gave you this handle? It's not a thing");

  return configs.at(handle);
}

}
