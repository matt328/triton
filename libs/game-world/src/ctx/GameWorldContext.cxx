#include "gw/GameWorldContext.hpp"

#include <di.hpp>

namespace di = boost::di;

namespace tr {

GameWorldContext::GameWorldContext(std::shared_ptr<IEventQueue> newEventQueue)
    : eventQueue{std::move(newEventQueue)} {
}

auto GameWorldContext::create(std::shared_ptr<IEventQueue> newEventQueue)
    -> std::shared_ptr<GameWorldContext> {
  const auto injector = di::make_injector(di::bind<IEventQueue>.to<>(newEventQueue));

  return injector.create<std::shared_ptr<GameWorldContext>>();
}

auto GameWorldContext::run() -> void {
}

auto GameWorldContext::stop() -> void {
}

}
