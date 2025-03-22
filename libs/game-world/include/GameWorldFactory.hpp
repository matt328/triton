#pragma once

namespace tr {

class IGameWorldContext;
class IAssetService;
class IEventBus;

class GameWorldFactory {
public:
  static auto createGameWorldContext(const std::shared_ptr<IEventBus>& eventBus,
                                     const std::shared_ptr<IAssetService>& assetService)
      -> std::shared_ptr<IGameWorldContext>;
};

}
