#pragma once

namespace tr {

class IGameWorldContext;
class IEventBus;
class IAssetService;
class IActionSystem;
class IResourceProxy;
class TaskQueue;
class GeometryGenerator;

auto createGameworldContext(const std::shared_ptr<IEventBus>& eventBus,
                            const std::shared_ptr<IAssetService>& assetService,
                            const std::shared_ptr<IActionSystem>& actionSystem,
                            const std::shared_ptr<IResourceProxy>& resourceProxy,
                            const std::shared_ptr<TaskQueue>& taskQueue,
                            const std::shared_ptr<GeometryGenerator>& geometryGenerator)
    -> std::shared_ptr<IGameWorldContext>;

}
