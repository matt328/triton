#pragma once

namespace tr {

class IGameWorldSystem;
class IGameObjectProxy;
class ITerrainSystemProxy;
class IWidgetService;

class IGameWorldContext {
public:
  IGameWorldContext() = default;
  virtual ~IGameWorldContext() = default;

  IGameWorldContext(const IGameWorldContext&) = default;
  IGameWorldContext(IGameWorldContext&&) = delete;
  auto operator=(const IGameWorldContext&) -> IGameWorldContext& = default;
  auto operator=(IGameWorldContext&&) -> IGameWorldContext& = delete;

  virtual auto getGameWorldSystem() -> std::shared_ptr<IGameWorldSystem> = 0;
  virtual auto getGameObjectProxy() -> std::shared_ptr<IGameObjectProxy> = 0;
  virtual auto getWidgetService() -> std::shared_ptr<IWidgetService> = 0;

  virtual auto registerTerrainProxy(std::shared_ptr<ITerrainSystemProxy> terrainProxy) -> void = 0;
};
}
