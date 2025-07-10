#pragma once

#include "api/fx/IEventQueue.hpp"

namespace tr {

/*
  This class will need to evolve over time in a number of ways, but for right now, it supports
  broadcast within a channel, meaning multiple handlers are able to handle the same event, with the
  caveat that the events are destroyed after the handler functions finish executing.

  If we ever need the begin/end batches to span multiple calls to dispatchEvents(), we can
  add support for handlers having to take ownership of the event as they handle it. This enables
  batches to span dispatchEvents(), at the cost of becoming point to point, where only one handler
  can respond to an event per channel. I don't know enough about how the use cases will shake out to
  decide if batches will span calls to dispatchEvents(). As soon as I say they don't need to, I'll
  find out they definitely need to.

  This class is basically the core of the entire engine, so it needs to be as efficient as possible.
  TODO(matt): figure out how to get rid of those locks.
*/
class [[deprecated("This is Deprecated")]] EventQueue : public IEventQueue {
public:
  EventQueue() = default;
  ~EventQueue() override {
    Log.trace("Destroying EventQueue");
  }

  EventQueue(const EventQueue&) = delete;
  EventQueue(EventQueue&&) = delete;
  auto operator=(const EventQueue&) -> EventQueue& = delete;
  auto operator=(EventQueue&&) -> EventQueue& = delete;

  void subscribe(std::type_index type,
                 std::function<void(const EventVariant&)> listener,
                 std::string channel) override {
    auto& handlerMap = getThreadLocalHandlers();
    std::lock_guard lock(handlerMap.mutex);
    handlerMap.handlers[type][channel].emplace_back(std::move(listener));
  }

  void emit(std::type_index type, EventVariant event, std::string channel) override {
    const std::lock_guard lock(emitMutex);
    eventQueues[channel].emplace_back(type, std::move(event));
  }

  void dispatchPending() override {
    auto& tlh = getThreadLocalHandlers();
    auto& dispatchQueue = tlh.dispatchQueue;
    auto& handlers = tlh.handlers;
    {
      std::lock_guard lock(emitMutex);

      for (auto& [channel, eventQueue] : eventQueues) {
        // Move matching events to this thread's dispatch queue
        auto it = eventQueue.begin();
        while (it != eventQueue.end()) {
          const auto& [type, event] = *it;
          if (handlers.contains(type)) {
            dispatchQueue.emplace_back(type, std::move(const_cast<EventVariant&>(event)));
            it = eventQueue.erase(it);
          } else {
            ++it;
          }
        }
      }
    }
    // Dispatch from the thread-local dispatch queue
    for (auto& [type, event] : dispatchQueue) {
      auto hit = handlers.find(type);
      if (hit != handlers.end()) {
        for (auto& [channelName, listeners] : hit->second) {
          for (auto& listener : listeners) {
            listener(event);
          }
        }
      }
    }
    dispatchQueue.clear();
  }

private:
  struct ThreadLocalHandlerMap {
    std::mutex mutex;
    std::unordered_map<
        std::type_index,
        std::unordered_map<std::string, std::vector<std::function<void(const EventVariant&)>>>>
        handlers;
    std::deque<std::pair<std::type_index, EventVariant>> dispatchQueue;
  };

  static auto getThreadLocalHandlers() -> ThreadLocalHandlerMap& {
    thread_local ThreadLocalHandlerMap handlerMap;
    return handlerMap;
  }

  std::mutex emitMutex;
  std::unordered_map<std::string, std::deque<std::pair<std::type_index, EventVariant>>> eventQueues;
};

}
