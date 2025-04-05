#pragma once

#include "bk/Rando.hpp"
#include "dd/RenderConfig.hpp"
#include "dd/RenderConfigHandle.hpp"

namespace tr {

class RenderConfigRegistry {
public:
  RenderConfigRegistry() = default;
  ~RenderConfigRegistry() = default;

  RenderConfigRegistry(const RenderConfigRegistry&) = delete;
  RenderConfigRegistry(RenderConfigRegistry&&) = delete;
  auto operator=(const RenderConfigRegistry&) -> RenderConfigRegistry& = delete;
  auto operator=(RenderConfigRegistry&&) -> RenderConfigRegistry& = delete;

  auto registerOrGet(const RenderConfig& target) -> RenderConfigHandle;
  [[nodiscard]] auto get(RenderConfigHandle handle) const -> const RenderConfig&;

private:
  MapKey configKeygen;
  std::unordered_map<RenderConfigHandle, RenderConfig> configs;
};

}
