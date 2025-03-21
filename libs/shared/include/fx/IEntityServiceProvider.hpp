#pragma once

#include "cm/EntitySystemTypes.hpp"
#include "cm/TransformData.hpp"

namespace tr {

class IEntityServiceProxy {
public:
  IEntityServiceProxy() = default;
  virtual ~IEntityServiceProxy() = default;

  IEntityServiceProxy(const IEntityServiceProxy&) = default;
  IEntityServiceProxy(IEntityServiceProxy&&) = delete;
  auto operator=(const IEntityServiceProxy&) -> IEntityServiceProxy& = default;
  auto operator=(IEntityServiceProxy&&) -> IEntityServiceProxy& = delete;

  virtual auto removeEntity(tr::EntityType entityType) -> void = 0;
  virtual auto setTransform(tr::EntityType entityType, TransformData transformData) -> void = 0;
};

}
