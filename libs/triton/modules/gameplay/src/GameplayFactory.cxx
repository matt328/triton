#include "gp/GameplayFactory.hpp"

#include "gp/DefaultGameplaySystem.hpp"

#include "GameplayContextImpl.hpp"

#include <di.hpp>

namespace di = boost::di;

namespace tr {

auto ComponentFactory::getContext(std::shared_ptr<IEventBus> eventBus)
    -> std::shared_ptr<IGameplayContext> {
  const auto injector = di::make_injector(di::bind<IGameplaySystem>.to<DefaultGameplaySystem>(),
                                          di::bind<IEventBus>.to(eventBus));

  return injector.create<std::shared_ptr<GameplayContextImpl>>();
}
}
