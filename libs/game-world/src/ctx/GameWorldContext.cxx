#include "gw/GameWorldContext.hpp"
#include "EntityManager.hpp"
#include "api/fx/IEventQueue.hpp"
#include "api/fx/IStateBuffer.hpp"
#include "gw/IEntityManager.hpp"
#include "api/gw/EditorStateBuffer.hpp"

#include <di.hpp>

namespace di = boost::di;

namespace tr {

GameWorldContext::GameWorldContext(std::shared_ptr<IEventQueue> newEventQueue,
                                   std::shared_ptr<IEntityManager> newEntityManager,
                                   std::shared_ptr<IStateBuffer> newStateBuffer)
    : eventQueue{std::move(newEventQueue)},
      entityManager{std::move(newEntityManager)},
      stateBuffer{std::move(newStateBuffer)} {
}

auto GameWorldContext::create(std::shared_ptr<IEventQueue> newEventQueue,
                              std::shared_ptr<IStateBuffer> newStateBuffer,
                              std::shared_ptr<EditorStateBuffer> newEditorStateBuffer)
    -> std::shared_ptr<GameWorldContext> {

  const auto injector = di::make_injector(di::bind<IEventQueue>.to<>(newEventQueue),
                                          di::bind<IEntityManager>.to<EntityManager>(),
                                          di::bind<IStateBuffer>.to<>(newStateBuffer),
                                          di::bind<EditorStateBuffer>.to<>(newEditorStateBuffer));

  return injector.create<std::shared_ptr<GameWorldContext>>();
}

auto GameWorldContext::run(std::stop_token token) -> void {
  Log.trace("Starting GameworldContext");
  using clock = std::chrono::steady_clock;
  constexpr int targetHz = 240;
  constexpr auto timestep = std::chrono::nanoseconds(1'000'000'000 / targetHz);

  auto nextTick = clock::now();

  while (!token.stop_requested()) {
    ZoneScopedN("Gameworld Loop");
    auto now = clock::now();
    if (now >= nextTick) {

      {
        ZoneScopedN("event dispatch");
        eventQueue->dispatchPending();
      }
      {
        ZoneScopedN("entityManager update");
        entityManager->update();
      }

      nextTick += timestep;

      // If we're falling behind, skip ahead to catch up (optional)
      if (now > nextTick + timestep * 10) {
        nextTick = now;
      }
    } else {
      ZoneScopedN("sleep");
      std::this_thread::sleep_until(nextTick);
    }
  }
}

}
