#include "gw/GameWorldFactory.hpp"
#include "GameWorldContextImpl.hpp"
#include "DefaultGameplaySystem.hpp"
#include "EntityService.hpp"
#include "gw/GameObjectProxyImpl.hpp"
// These includes have to be here or else DI won't work. Should explicitly register everything.
// then they have a reason to be here and nothing is magickal.
#include "systems/CameraSystem.hpp"
#include "systems/TransformSystem.hpp"
#include "systems/AnimationSystem.hpp"
#include "systems/RenderDataSystem.hpp"

#include <di.hpp>

namespace di = boost::di;

namespace tr {

auto createGameWorldContext(const std::shared_ptr<IEventBus>& eventBus,
                            const std::shared_ptr<IAssetService>& assetService,
                            const std::shared_ptr<IActionSystem>& actionSystem,
                            const std::shared_ptr<ITerrainSystemProxy>& terrainSystemProxy,
                            const std::shared_ptr<IResourceProxy>& resourceProxy)
    -> std::shared_ptr<IGameWorldContext> {

  const auto injector = di::make_injector(di::bind<IGameWorldSystem>.to<DefaultGameplaySystem>(),
                                          di::bind<IAssetService>.to<>(assetService),
                                          di::bind<IEventBus>.to<>(eventBus),
                                          di::bind<IActionSystem>.to<>(actionSystem),
                                          di::bind<ITerrainSystemProxy>.to<>(terrainSystemProxy),
                                          di::bind<IResourceProxy>.to<>(resourceProxy),
                                          di::bind<IGameObjectProxy>.to<GameObjectProxyImpl>());

  return injector.create<std::shared_ptr<GameWorldContextImpl>>();
}

}
