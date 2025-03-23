#pragma once

namespace tr {

class ITerrainSystemProxy;

class ITerrainContext {
public:
  ITerrainContext() = default;
  virtual ~ITerrainContext() = default;

  ITerrainContext(const ITerrainContext&) = default;
  ITerrainContext(ITerrainContext&&) = delete;
  auto operator=(const ITerrainContext&) -> ITerrainContext& = default;
  auto operator=(ITerrainContext&&) -> ITerrainContext& = delete;

  virtual auto getTerrainSystemProxy() -> std::shared_ptr<ITerrainSystemProxy> = 0;
};

}
