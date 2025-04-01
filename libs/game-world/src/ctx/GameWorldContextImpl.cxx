#include "GameWorldContextImpl.hpp"

namespace tr {

GameWorldContextImpl::GameWorldContextImpl(std::shared_ptr<IGameWorldSystem> newGameWorldSystem,
                                           std::shared_ptr<IGameObjectProxy> newGameObjectProxy,
                                           std::shared_ptr<IWidgetService> newWidgetService)
    : gameWorldSystem{std::move(newGameWorldSystem)},
      gameObjectProxy{std::move(newGameObjectProxy)},
      widgetService{std::move(newWidgetService)} {
}

auto GameWorldContextImpl::getGameWorldSystem() -> std::shared_ptr<IGameWorldSystem> {
  return gameWorldSystem;
}

auto GameWorldContextImpl::getGameObjectProxy() -> std::shared_ptr<IGameObjectProxy> {
  return gameObjectProxy;
}

auto GameWorldContextImpl::getWidgetService() -> std::shared_ptr<IWidgetService> {
  return widgetService;
}

auto GameWorldContextImpl::registerTerrainProxy(std::shared_ptr<ITerrainSystemProxy> terrainProxy)
    -> void {
  this->terrainProxy = terrainProxy;
}

}
