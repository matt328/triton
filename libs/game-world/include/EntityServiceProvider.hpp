#pragma once

#include "fx/ext/IGameObjectProxy.hpp"

#include "cm/TransformData.hpp"

namespace tr {

class EntityService;

class EntityServiceProvider : public IGameObjectProxy {
public:
  explicit EntityServiceProvider(std::shared_ptr<EntityService> newEntityService);
  ~EntityServiceProvider() override = default;

  EntityServiceProvider(const EntityServiceProvider&) = default;
  EntityServiceProvider(EntityServiceProvider&&) = delete;
  auto operator=(const EntityServiceProvider&) -> EntityServiceProvider& = default;
  auto operator=(EntityServiceProvider&&) -> EntityServiceProvider& = delete;

  auto removeEntity(GameObjectId entityType) -> void override;
  auto setTransform(GameObjectId entityType, TransformData transformData) -> void override;

private:
  std::shared_ptr<EntityService> entityService;
};

}
