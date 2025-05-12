#include "gfx/GraphicsContext.hpp"

#include <di.hpp>

namespace di = boost::di;

namespace tr {

GraphicsContext::GraphicsContext(std::shared_ptr<IEventQueue> newEventQueue)
    : eventQueue{std::move(newEventQueue)} {
}

auto GraphicsContext::create(std::shared_ptr<IEventQueue> newEventQueue)
    -> std::shared_ptr<GraphicsContext> {
  const auto injector = di::make_injector(di::bind<IEventQueue>.to<>(newEventQueue));
  return injector.create<std::shared_ptr<GraphicsContext>>();
}

auto GraphicsContext::run() -> void {
}

auto GraphicsContext::stop() -> void {
}

}
