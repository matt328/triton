#pragma once

#include "api/fx/IGameWorldContext.hpp"

namespace tr {

class IGameplaySystem;
class IDebugRegistry;

class GameWorldContextImpl : public IGameWorldContext {
public:
  explicit GameWorldContextImpl(std::shared_ptr<IGameWorldSystem> newGameWorldSystem,
                                std::shared_ptr<IGameObjectProxy> newGameObjectProxy,
                                std::shared_ptr<IDebugRegistry> newDebugRegistry);
  ~GameWorldContextImpl() override = default;

  GameWorldContextImpl(const GameWorldContextImpl&) = default;
  GameWorldContextImpl(GameWorldContextImpl&&) = delete;
  auto operator=(const GameWorldContextImpl&) -> GameWorldContextImpl& = default;
  auto operator=(GameWorldContextImpl&&) -> GameWorldContextImpl& = delete;

  auto getGameWorldSystem() -> std::shared_ptr<IGameWorldSystem> override;
  auto getGameObjectProxy() -> std::shared_ptr<IGameObjectProxy> override;
  auto getDebugRegistry() -> std::shared_ptr<IDebugRegistry> override;
  auto registerTerrainProxy(std::shared_ptr<ITerrainSystemProxy> terrainProxy) -> void override;

private:
  std::shared_ptr<IGameWorldSystem> gameWorldSystem;
  std::shared_ptr<IGameObjectProxy> gameObjectProxy;
  std::shared_ptr<IDebugRegistry> debugRegistry;

  std::shared_ptr<ITerrainSystemProxy> terrainProxy;
};

}
