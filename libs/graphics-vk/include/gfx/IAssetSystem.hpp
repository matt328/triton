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

  virtual auto run(std::stop_token token) -> void = 0;
};

}
