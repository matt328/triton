#include "fx/FrameworkFactory.hpp"
#include "FixedGameLoop.hpp"
#include "FrameworkContextImpl.hpp"
#include "fx/IGraphicsContext.hpp"
#include "DefaultEventBus.hpp"
#include "fx/GuiCallbackRegistrar.hpp"

#include "VkGraphicsFactory.hpp"

#include "TaskQueue.hpp"

#include <di.hpp>

namespace di = boost::di;

namespace tr {

auto createFrameworkContext([[maybe_unused]] const FrameworkConfig& config)
    -> std::shared_ptr<IFrameworkContext> {

  const auto guiCallbackRegistrar = std::make_shared<GuiCallBackRegistrar>();

  const auto injector =
      di::make_injector(di::bind<IGameLoop>.to<FixedGameLoop>(),
                        di::bind<IEventBus>.to<DefaultEventBus>(),
                        di::bind<ITaskQueue>.to<TaskQueue>(),
                        di::bind<IEntityServiceProvider>.to<EntityServiceProvider>(),
                        di::bind<IGuiCallbackRegistrar>.to(guiCallbackRegistrar),
                        di::bind<IGraphicsContext>.to([&guiCallbackRegistrar]() {
                          return createVkGraphicsContext(guiCallbackRegistrar);
                        }));

  auto gameLoop = std::make_shared<FixedGameLoop>();

  const auto frameworkContext = injector.create<std::shared_ptr<FrameworkContextImpl>>();

  return frameworkContext;
}

}
