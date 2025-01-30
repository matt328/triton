#pragma once

namespace ed {

class AssetTool {
public:
  AssetTool();
  ~AssetTool();

  AssetTool(const AssetTool&) = delete;
  AssetTool(AssetTool&&) = delete;
  auto operator=(const AssetTool&) -> AssetTool& = delete;
  auto operator=(AssetTool&&) -> AssetTool& = delete;

  void render();

  static constexpr auto ComponentName = "Asset Tool";
};

}
