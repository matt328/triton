#pragma once

#include "cm/EntitySystemTypes.hpp"
#include "cm/TransformData.hpp"

namespace tr {

class IGameObjectProxy {
public:
  IGameObjectProxy() = default;
  virtual ~IGameObjectProxy() = default;

  IGameObjectProxy(const IGameObjectProxy&) = default;
  IGameObjectProxy(IGameObjectProxy&&) = delete;
  auto operator=(const IGameObjectProxy&) -> IGameObjectProxy& = default;
  auto operator=(IGameObjectProxy&&) -> IGameObjectProxy& = delete;

  virtual auto removeEntity(tr::EntityType entityType) -> void = 0;
  virtual auto setTransform(tr::EntityType entityType, TransformData transformData) -> void = 0;
};

}
