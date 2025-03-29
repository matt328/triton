#pragma once

#include "api/ext/IGameObjectProxy.hpp"

namespace tr {

class ITerrainSystemProxy;
class IResourceProxy;

class ITerrainContext {
public:
  ITerrainContext() = default;
  virtual ~ITerrainContext() = default;

  ITerrainContext(const ITerrainContext&) = default;
  ITerrainContext(ITerrainContext&&) = delete;
  auto operator=(const ITerrainContext&) -> ITerrainContext& = default;
  auto operator=(ITerrainContext&&) -> ITerrainContext& = delete;

  virtual auto getTerrainSystemProxy() -> std::shared_ptr<ITerrainSystemProxy> = 0;

  virtual auto registerGameObjectProxy(std::shared_ptr<IGameObjectProxy> newGameObjectProxy)
      -> void = 0;

  virtual auto registerResourceProxy(std::shared_ptr<IResourceProxy> newResourceProxy) -> void = 0;
};

}
