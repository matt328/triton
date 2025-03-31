#include "GameWorldContextImpl.hpp"

namespace tr {

GameWorldContextImpl::GameWorldContextImpl(std::shared_ptr<IGameWorldSystem> newGameWorldSystem,
                                           std::shared_ptr<IGameObjectProxy> newGameObjectProxy,
                                           std::shared_ptr<IDebugRegistry> newDebugRegistry)
    : gameWorldSystem{std::move(newGameWorldSystem)},
      gameObjectProxy{std::move(newGameObjectProxy)},
      debugRegistry{std::move(newDebugRegistry)} {
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

auto GameWorldContextImpl::getDebugRegistry() -> std::shared_ptr<IDebugRegistry> {
  return debugRegistry;
}

}
