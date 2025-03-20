#include "gw/GameworldFactory.hpp"
#include "GameworldContextImpl.hpp"
#include "DefaultGameplaySystem.hpp"

#include <di.hpp>

namespace di = boost::di;

namespace tr {

auto createGameworldContext(std::shared_ptr<IEventBus> eventBus)
    -> std::shared_ptr<IGameworldContext> {
  const auto injector = di::make_injector(di::bind<IGameplaySystem>.to<DefaultGameplaySystem>(),
                                          di::bind<IEventBus>.to<>(eventBus));

  return injector.create<std::shared_ptr<GameworldContextImpl>>();
}

}
