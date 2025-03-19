#include "fx/FrameworkFactory.hpp"
#include "FixedGameLoop.hpp"
#include "FrameworkContextImpl.hpp"
#include "fx/IGraphicsContext.hpp"
#include "fx/GuiCallbackRegistrar.hpp"

#include "VkGraphicsFactory.hpp"

#include <di.hpp>

namespace di = boost::di;

namespace tr {

auto createFrameworkContext([[maybe_unused]] const FrameworkConfig& config)
    -> std::shared_ptr<IFrameworkContext> {

  const auto guiCallbackRegistrar = std::make_shared<GuiCallBackRegistrar>();

  const auto injector = di::make_injector(di::bind<IGameLoop>.to<FixedGameLoop>(),
                                          di::bind<IGuiCallbackRegistrar>.to(guiCallbackRegistrar),
                                          di::bind<IGraphicsContext>.to([&guiCallbackRegistrar]() {
                                            return createVkGraphicsContext(guiCallbackRegistrar);
                                          }));

  auto gameLoop = std::make_shared<FixedGameLoop>();

  auto* frameworkContextImpl = new FrameworkContextImpl(gameLoop, guiCallbackRegistrar);

  const auto frameworkContext = injector.create<std::shared_ptr<FrameworkContextImpl>>();

  return frameworkContext;
}

}
