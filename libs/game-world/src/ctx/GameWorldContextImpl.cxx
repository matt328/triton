#include "GameWorldContextImpl.hpp"

namespace tr {

GameWorldContextImpl::GameWorldContextImpl(std::shared_ptr<IGameWorldSystem> newGameWorldSystem,
                                           std::shared_ptr<IGameObjectProxy> newGameObjectProxy)
    : gameWorldSystem{std::move(newGameWorldSystem)},
      gameObjectProxy{std::move(newGameObjectProxy)} {
}

auto GameWorldContextImpl::getGameWorldSystem() -> std::shared_ptr<IGameWorldSystem> {
  return gameWorldSystem;
}

auto GameWorldContextImpl::getGameObjectProxy() -> std::shared_ptr<IGameObjectProxy> {
  return gameObjectProxy;
}

auto GameWorldContextImpl::registerTerrainProxy(std::shared_ptr<ITerrainSystemProxy> terrainProxy)
    -> void {
  this->terrainProxy = terrainProxy;
}

}
