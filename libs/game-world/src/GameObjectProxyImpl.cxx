#include "gw/GameObjectProxyImpl.hpp"

namespace tr {

GameObjectProxyImpl::GameObjectProxyImpl(std::shared_ptr<EntityService> newEntityService)
    : entityService{std::move(newEntityService)} {
}

auto GameObjectProxyImpl::removeEntity([[maybe_unused]] GameObjectId entityType) -> void {
}

auto GameObjectProxyImpl::setTransform([[maybe_unused]] GameObjectId entityType,
                                       [[maybe_unused]] TransformData transformData) -> void {
}

auto GameObjectProxyImpl::getEditableGameObjects() -> std::vector<GameObjectId> {

  return {};
}

auto GameObjectProxyImpl::getStaticObjectDetails() -> StaticObjectDetails {
  return StaticObjectDetails{};
}

auto GameObjectProxyImpl::getDynamicObjectDetails() -> DynamicObjectDetails {
  return DynamicObjectDetails{};
}

auto GameObjectProxyImpl::getTerrainObjectDetails() -> TerrainObjectDetails {
  return TerrainObjectDetails{};
}

auto GameObjectProxyImpl::setStaticObjectDetails(
    [[maybe_unused]] GameObjectId id,
    [[maybe_unused]] const StaticObjectDetails& details) -> void {
}

auto GameObjectProxyImpl::setDynamicObjectDetails(
    [[maybe_unused]] GameObjectId id,
    [[maybe_unused]] const DynamicObjectDetails& details) -> void {
}

auto GameObjectProxyImpl::setTerrainObjectDetails(
    [[maybe_unused]] GameObjectId id,
    [[maybe_unused]] const TerrainObjectDetails& details) -> void {
}

auto GameObjectProxyImpl::triangulateChunk([[maybe_unused]] GameObjectId terrainId,
                                           [[maybe_unused]] GameObjectId chunkId,
                                           [[maybe_unused]] glm::ivec3 cellPosition) -> void {
}
}
