#pragma once

namespace tr {

class IGameWorldContext;
class IEventBus;
class IAssetService;
class IActionSystem;
class ITerrainSystemProxy;
class IResourceProxy;

auto createGameworldContext(const std::shared_ptr<IEventBus>& eventBus,
                            const std::shared_ptr<IAssetService>& assetService,
                            const std::shared_ptr<IActionSystem>& actionSystem,
                            const std::shared_ptr<ITerrainSystemProxy>& terrainSystemProxy,
                            const std::shared_ptr<IResourceProxy>& resourceProxy)
    -> std::shared_ptr<IGameWorldContext>;

}
