#include "gw/GameWorldContext.hpp"
#include "EntityManager.hpp"
#include "api/fx/IEventQueue.hpp"
#include "gw/IEntityManager.hpp"

#include <di.hpp>

namespace di = boost::di;

namespace tr {

GameWorldContext::GameWorldContext(std::shared_ptr<IEventQueue> newEventQueue,
                                   std::shared_ptr<IEntityManager> newEntityManager)
    : eventQueue{std::move(newEventQueue)}, entityManager{std::move(newEntityManager)} {
}

auto GameWorldContext::create(std::shared_ptr<IEventQueue> newEventQueue)
    -> std::shared_ptr<GameWorldContext> {

  const auto injector = di::make_injector(di::bind<IEventQueue>.to<>(newEventQueue),
                                          di::bind<IEntityManager>.to<EntityManager>());

  return injector.create<std::shared_ptr<GameWorldContext>>();
}

auto GameWorldContext::run() -> void {
  Log.trace("Starting GameworldContext");
  using clock = std::chrono::steady_clock;
  constexpr int targetHz = 240;
  constexpr auto timestep = std::chrono::nanoseconds(1'000'000'000 / targetHz);

  auto nextTick = clock::now();

  running = true;
  while (running) {
    auto now = clock::now();
    if (now >= nextTick) {

      eventQueue->dispatchPending();
      entityManager->update();

      nextTick += timestep;

      // If we're falling behind, skip ahead to catch up (optional)
      if (now > nextTick + timestep * 10) {
        nextTick = now;
      }
    } else {
      std::this_thread::sleep_until(nextTick);
    }
  }
}

auto GameWorldContext::stop() -> void {
  Log.trace("GameWorldContext::stop()");
  running = false;
}

}
