#pragma once

namespace tr {

class IAssetSystem {
public:
  IAssetSystem() = default;
  virtual ~IAssetSystem() = default;

  IAssetSystem(const IAssetSystem&) = default;
  IAssetSystem(IAssetSystem&&) = delete;
  auto operator=(const IAssetSystem&) -> IAssetSystem& = default;
  auto operator=(IAssetSystem&&) -> IAssetSystem& = delete;

  virtual auto run() -> void = 0;
  virtual auto requestStop() -> void = 0;
};

}
