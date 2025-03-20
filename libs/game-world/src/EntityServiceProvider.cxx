#include "EntityServiceProvider.hpp"
#include "EntityService.hpp"

namespace tr {
EntityServiceProvider::EntityServiceProvider(std::shared_ptr<EntityService> newEntityService)
    : entityService{std::move(newEntityService)} {
}

auto EntityServiceProvider::removeEntity(tr::EntityType entityType) -> void {
  entityService->removeEntity(entityType);
}

auto EntityServiceProvider::setTransform(tr::EntityType entityType,
                                         TransformData transformData) -> void {
  entityService->setTransform(entityType, transformData);
}

}
