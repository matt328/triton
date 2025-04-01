#pragma once

#include "api/fx/IGameWorldContext.hpp"

namespace tr {

class IGameplaySystem;
class IWidgetService;

class GameWorldContextImpl : public IGameWorldContext {
public:
  explicit GameWorldContextImpl(std::shared_ptr<IGameWorldSystem> newGameWorldSystem,
                                std::shared_ptr<IGameObjectProxy> newGameObjectProxy,
                                std::shared_ptr<IWidgetService> newWidgetService);
  ~GameWorldContextImpl() override = default;

  GameWorldContextImpl(const GameWorldContextImpl&) = default;
  GameWorldContextImpl(GameWorldContextImpl&&) = delete;
  auto operator=(const GameWorldContextImpl&) -> GameWorldContextImpl& = default;
  auto operator=(GameWorldContextImpl&&) -> GameWorldContextImpl& = delete;

  auto getGameWorldSystem() -> std::shared_ptr<IGameWorldSystem> override;
  auto getGameObjectProxy() -> std::shared_ptr<IGameObjectProxy> override;
  auto getWidgetService() -> std::shared_ptr<IWidgetService> override;
  auto registerTerrainProxy(std::shared_ptr<ITerrainSystemProxy> terrainProxy) -> void override;

private:
  std::shared_ptr<IGameWorldSystem> gameWorldSystem;
  std::shared_ptr<IGameObjectProxy> gameObjectProxy;
  std::shared_ptr<IWidgetService> widgetService;

  std::shared_ptr<ITerrainSystemProxy> terrainProxy;
};

}
