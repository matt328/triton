#include "fx/FrameworkFactory.hpp"
#include "DefaultAssetService.hpp"
#include "fx/IGraphicsContext.hpp"
#include "fx/IGameworldContext.hpp"
#include "FixedGameLoop.hpp"
#include "FrameworkContextImpl.hpp"
#include "DefaultEventBus.hpp"
#include "fx/GuiCallbackRegistrar.hpp"

#include "VkGraphicsFactory.hpp"
#include "gw/GameworldFactory.hpp"

#include "TaskQueue.hpp"

#include <di.hpp>

namespace di = boost::di;

namespace tr {

auto createFrameworkContext([[maybe_unused]] const FrameworkConfig& config)
    -> std::shared_ptr<IFrameworkContext> {

  const auto guiCallbackRegistrar = std::make_shared<GuiCallBackRegistrar>();
  const auto eventBus = std::make_shared<DefaultEventBus>();
  const auto assetService = std::make_shared<DefaultAssetService>();

  const auto injector =
      di::make_injector(di::bind<IEventBus>.to<>(eventBus),
                        di::bind<IAssetService>.to<>(assetService),
                        di::bind<IGameLoop>.to<FixedGameLoop>(),
                        di::bind<ITaskQueue>.to<TaskQueue>(),
                        di::bind<IEntityServiceProvider>.to<EntityServiceProvider>(),
                        di::bind<IGuiCallbackRegistrar>.to(guiCallbackRegistrar),
                        di::bind<IGraphicsContext>.to([&guiCallbackRegistrar]() {
                          return createVkGraphicsContext(guiCallbackRegistrar);
                        }));

  const auto frameworkContext = injector.create<std::shared_ptr<FrameworkContextImpl>>();

  const auto gameworldContext = createGameworldContext(eventBus, assetService);

  return frameworkContext;
}

}
