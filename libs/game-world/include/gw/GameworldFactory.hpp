#pragma once

namespace tr {

class IGameworldContext;
class IEventBus;
class IAssetService;

auto createGameworldContext(const std::shared_ptr<IEventBus>& eventBus,
                            const std::shared_ptr<IAssetService>& assetService)
    -> std::shared_ptr<IGameworldContext>;

}
