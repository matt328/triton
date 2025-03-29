#include "fx/FrameworkFactory.hpp"
#include "DefaultAssetService.hpp"
#include "IGuiSystem.hpp"
#include "api/ext/ITerrainSystemProxy.hpp"
#include "api/fx/IGameWorldContext.hpp"
#include "FixedGameLoop.hpp"
#include "FrameworkContextImpl.hpp"
#include "DefaultEventBus.hpp"
#include "api/fx/IGameWorldSystem.hpp"
#include "fx/GuiCallbackRegistrar.hpp"
#include "api/fx/IGraphicsContext.hpp"
#include "api/fx/ITerrainContext.hpp"
#include "api/ext/IGameObjectProxy.hpp"
#include "ActionSystem.hpp"
#include "api/action/IActionSystem.hpp"

#include "VkGraphicsFactory.hpp"
#include "gw/GameWorldFactory.hpp"
#include "VoxelTerrainFactory.hpp"
#include "gfx/IRenderContext.hpp"
#include "bk/TaskQueue.hpp"
#include "gfx/IWindow.hpp"

#define BOOST_DI_CFG_CTOR_LIMIT_SIZE 11
#include <di.hpp>

namespace di = boost::di;

namespace tr {

auto createFrameworkContext(const FrameworkConfig& config, std::shared_ptr<IGuiAdapter> guiAdapter)
    -> std::shared_ptr<IFrameworkContext> {

  const auto guiCallbackRegistrar = std::make_shared<GuiCallBackRegistrar>();
  const auto eventBus = std::make_shared<DefaultEventBus>();
  const auto assetService = std::make_shared<DefaultAssetService>();
  const auto actionSystem = std::make_shared<ActionSystem>(eventBus);
  const auto taskQueue = std::make_shared<TaskQueue>(TaskQueueConfig{.maxQueueSize = 1024});

  const auto graphicsConfig = VkGraphicsCreateInfo{.initialWindowSize = config.initialWindowSize,
                                                   .windowTitle = config.windowTitle};

  const auto graphicsContext = createVkGraphicsContext(graphicsConfig,
                                                       guiCallbackRegistrar,
                                                       eventBus,
                                                       taskQueue,
                                                       guiAdapter);

  const auto terrainContext = createTerrainContext();
  const auto terrainProxy = terrainContext->getTerrainSystemProxy();

  const auto gameWorldContext = createGameworldContext(eventBus,
                                                       assetService,
                                                       actionSystem,
                                                       graphicsContext->getResourceProxy());
  const auto gameObjectProxy = gameWorldContext->getGameObjectProxy();

  gameWorldContext->registerTerrainProxy(terrainProxy);
  terrainContext->registerGameObjectProxy(gameObjectProxy);
  terrainContext->registerResourceProxy(graphicsContext->getResourceProxy());

  const auto frameworkInjector = di::make_injector(
      di::bind<IGameLoop>.to<FixedGameLoop>(),
      di::bind<IRenderContext>.to<>(
          [&graphicsContext] { return graphicsContext->getRenderContext(); }),
      di::bind<IGuiCallbackRegistrar>.to(guiCallbackRegistrar),
      di::bind<TaskQueue>.to<>(taskQueue),
      di::bind<IEventBus>.to<>(eventBus),
      di::bind<IGameObjectProxy>.to<>(gameObjectProxy),
      di::bind<IGameWorldSystem>.to(
          [&gameWorldContext] { return gameWorldContext->getGameWorldSystem(); }),
      di::bind<IAssetService>.to<>(assetService),
      di::bind<IActionSystem>.to<>(actionSystem),
      di::bind<IGameWorldContext>.to<>(gameWorldContext),
      di::bind<IGraphicsContext>.to<>(graphicsContext),
      di::bind<IWindow>.to<>([&graphicsContext] { return graphicsContext->getWindow(); }),
      di::bind<IGuiSystem>.to<>([&graphicsContext] { return graphicsContext->getGuiSystem(); }),
      di::bind<ITerrainSystemProxy>.to<>(terrainProxy));

  const auto frameworkContext = frameworkInjector.create<std::shared_ptr<FrameworkContextImpl>>();

  return frameworkContext;
}

}
