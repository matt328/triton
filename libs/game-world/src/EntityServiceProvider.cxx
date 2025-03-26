#include "EntityServiceProvider.hpp"
#include "EntityService.hpp"
#include "EntitySystemTypes.hpp"

namespace tr {
EntityServiceProvider::EntityServiceProvider(std::shared_ptr<EntityService> newEntityService)
    : entityService{std::move(newEntityService)} {
}

auto EntityServiceProvider::removeEntity(GameObjectId entityType) -> void {
  entityService->removeEntity(static_cast<EntityType>(entityType));
}

auto EntityServiceProvider::setTransform(GameObjectId entityType,
                                         TransformData transformData) -> void {
  const auto transform = Transform{
      .rotation = transformData.rotation,
      .position = transformData.position,
  };
  entityService->setTransform(static_cast<EntityType>(entityType), transform);
}

}
