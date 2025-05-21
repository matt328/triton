#pragma once

#include "api/fx/IEventQueue.hpp"

namespace tr {

class EventQueue : public IEventQueue {
public:
  EventQueue() = default;
  ~EventQueue() override {
    Log.trace("Destroying EventQueue");
  }

  EventQueue(const EventQueue&) = delete;
  EventQueue(EventQueue&&) = delete;
  auto operator=(const EventQueue&) -> EventQueue& = delete;
  auto operator=(EventQueue&&) -> EventQueue& = delete;

  void subscribe(std::type_index type, std::function<void(const EventVariant&)> listener) override {
    auto& handlerMap = getThreadLocalHandlers();
    std::lock_guard lock(handlerMap.mutex);
    handlerMap.handlers[type].emplace_back(std::move(listener));
  }

  void emit(std::type_index type, EventVariant event) override {
    const std::lock_guard lock(emitMutex);
    eventQueue.emplace_back(type, std::move(event));
  }

  void dispatchPending() override {
    auto& tlh = getThreadLocalHandlers();
    {
      std::lock_guard lock(emitMutex);
      auto& dispatchQueue = tlh.dispatchQueue;
      auto& handlers = tlh.handlers;

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

    // Now dispatch from the thread-local dispatch queue
    auto& dispatchQueue = tlh.dispatchQueue;
    auto& handlers = tlh.handlers;

    for (auto& [type, event] : dispatchQueue) {
      auto it = handlers.find(type);
      if (it != handlers.end()) {
        for (auto& listener : it->second) {
          listener(event);
        }
      }
    }

    dispatchQueue.clear();
  }

private:
  struct ThreadLocalHandlerMap {
    std::mutex mutex;
    std::unordered_map<std::type_index, std::vector<std::function<void(const EventVariant&)>>>
        handlers;
    std::deque<std::pair<std::type_index, EventVariant>> dispatchQueue;
  };

  static auto getThreadLocalHandlers() -> ThreadLocalHandlerMap& {
    thread_local ThreadLocalHandlerMap handlerMap;
    return handlerMap;
  }

  std::mutex emitMutex;
  std::deque<std::pair<std::type_index, EventVariant>> eventQueue;
};

}
