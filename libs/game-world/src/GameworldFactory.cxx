#include "GameWorldFactory.hpp"
#include "GameWorldContextImpl.hpp"
#include "DefaultGameplaySystem.hpp"

#include <di.hpp>

namespace di = boost::di;

namespace tr {

auto createGameWorldContext(const std::shared_ptr<IEventBus>& eventBus,
                            const std::shared_ptr<IAssetService>& assetService)
    -> std::shared_ptr<IGameWorldContext> {
  const auto injector = di::make_injector(di::bind<IGameWorldSystem>.to<DefaultGameplaySystem>(),
                                          di::bind<IAssetService>.to<>(assetService),
                                          di::bind<IEventBus>.to<>(eventBus),
                                          di::bind<IActionSystem>.to<>(ActionSystem));

  return injector.create<std::shared_ptr<GameWorldContextImpl>>();
}

}
