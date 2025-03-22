#include "fx/FrameworkFactory.hpp"
#include "DefaultAssetService.hpp"
#include "fx/IGameWorldContext.hpp"
#include "FixedGameLoop.hpp"
#include "FrameworkContextImpl.hpp"
#include "DefaultEventBus.hpp"
#include "fx/GuiCallbackRegistrar.hpp"
#include "fx/IGameObjectProxy.hpp"

#include "VkGraphicsFactory.hpp"
#include "gw/GameworldFactory.hpp"

#include "TaskQueue.hpp"

#include <di.hpp>

namespace di = boost::di;

namespace tr {

auto createFrameworkContext([[maybe_unused]] const FrameworkConfig& config)
    -> std::shared_ptr<IFrameworkContext> {

  /*
    The Framework should provide an interface for external clients to crud game objects, and the
    GameWorld library will provide the impl.

    The FrameworkContext will own and provide access to any systems via Interfaces, with the actual
    impls provided by the separate libs, and being completely unknown to the application.

    FrameworkContext will create its own injector, the GameWorldContext, and the GraphicsContext.
    GameWorldContext and GraphicsContext will have their own injectors, but that is an internal
    detail of the libs, they might not use boost di.

    Each injector should explicitly di::bind all of its dependencies, don't use automatic injection.
    This will help document all the components in the context.
   */

  const auto guiCallbackRegistrar = std::make_shared<GuiCallBackRegistrar>();
  const auto eventBus = std::make_shared<DefaultEventBus>();
  const auto assetService = std::make_shared<DefaultAssetService>();

  // This might seem odd that we pull some things out of the contexts to bind them, then just bind
  // the context anyway, but we need to store off the context so it doesn't go out of scope, and
  // bind components individually to maintain some modularity.
  const auto gameWorldContext = createGameworldContext(eventBus, assetService);
  const auto gameObjectProxy = gameWorldContext->getGameObjectProxy();
  const auto gameWorldSystem = gameWorldContext->getGameWorldSystem();

  const auto graphicsContext = createVkGraphicsContext(guiCallbackRegistrar);

  const auto frameworkInjector =
      di::make_injector(di::bind<IGameLoop>.to<FixedGameLoop>(),
                        di::bind<IGuiCallbackRegistrar>.to(guiCallbackRegistrar),
                        di::bind<ITaskQueue>.to<TaskQueue>(),
                        di::bind<IEventBus>.to<>(eventBus),
                        di::bind<IGameObjectProxy>.to<>(gameObjectProxy),
                        di::bind<IGameWorldSystem>.to(gameWorldSystem),
                        di::bind<IAssetService>.to<>(assetService),
                        di::bind<IGameWorldContext>.to<>(gameWorldContext),
                        di::bind<IGraphicsContext>.to<>(graphicsContext));

  const auto frameworkContext = frameworkInjector.create<std::shared_ptr<FrameworkContextImpl>>();

  return frameworkContext;
}

}
