#pragma once

#include "fx/ext/IGameObjectProxy.hpp"
#include "EntityService.hpp"

namespace tr {

class GameObjectProxyImpl : public IGameObjectProxy {
public:
  explicit GameObjectProxyImpl(std::shared_ptr<EntityService> newEntityService);
  ~GameObjectProxyImpl() override = default;

  GameObjectProxyImpl(const GameObjectProxyImpl&) = delete;
  GameObjectProxyImpl(GameObjectProxyImpl&&) = delete;
  auto operator=(const GameObjectProxyImpl&) -> GameObjectProxyImpl& = delete;
  auto operator=(GameObjectProxyImpl&&) -> GameObjectProxyImpl& = delete;

  auto removeEntity(tr::EntityType entityType) -> void override;
  auto setTransform(tr::EntityType entityType, TransformData transformData) -> void override;

  auto getEditableGameObjects() -> std::vector<GameObjectId> override;
  auto getStaticObjectDetails() -> StaticObjectDetails override;
  auto getDynamicObjectDetails() -> DynamicObjectDetails override;
  auto getTerrainObjectDetails() -> TerrainObjectDetails override;

  auto setStaticObjectDetails(GameObjectId id, const StaticObjectDetails& details) -> void override;
  auto setDynamicObjectDetails(GameObjectId id,
                               const DynamicObjectDetails& details) -> void override;
  auto setTerrainObjectDetails(GameObjectId id,
                               const TerrainObjectDetails& details) -> void override;

  auto triangulateChunk(GameObjectId terrainId,
                        GameObjectId chunkId,
                        glm::ivec3 cellPosition) -> void override;

private:
  std::shared_ptr<EntityService> entityService;
};

}
