#pragma once

namespace tr {

class IGameWorldContext;
class IEventBus;
class IAssetService;
class IActionSystem;
class IResourceProxy;
class TaskQueue;

auto createGameworldContext(const std::shared_ptr<IEventBus>& eventBus,
                            const std::shared_ptr<IAssetService>& assetService,
                            const std::shared_ptr<IActionSystem>& actionSystem,
                            const std::shared_ptr<IResourceProxy>& resourceProxy,
                            const std::shared_ptr<TaskQueue>& taskQueue)
    -> std::shared_ptr<IGameWorldContext>;

}
