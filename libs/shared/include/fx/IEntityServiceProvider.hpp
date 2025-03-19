#pragma once

#include "cm/EntitySystemTypes.hpp"
#include "cm/TransformData.hpp"

namespace tr {

class IEntityServiceProvider {
public:
  IEntityServiceProvider() = default;
  virtual ~IEntityServiceProvider() = default;

  IEntityServiceProvider(const IEntityServiceProvider&) = default;
  IEntityServiceProvider(IEntityServiceProvider&&) = delete;
  auto operator=(const IEntityServiceProvider&) -> IEntityServiceProvider& = default;
  auto operator=(IEntityServiceProvider&&) -> IEntityServiceProvider& = delete;

  virtual auto removeEntity(tr::EntityType entityType) -> void = 0;
  virtual auto setTransform(tr::EntityType entityType, TransformData transformData) -> void = 0;
};

}
