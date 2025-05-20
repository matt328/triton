#pragma once

#include "api/fx/IEventQueue.hpp"

namespace tr {

class EventQueue : public IEventQueue {
public:
  EventQueue() = default;
  ~EventQueue() override = default;

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
    auto& handlerMap = getThreadLocalHandlers();
    std::deque<std::pair<std::type_index, EventVariant>> queueCopy;

    {
      std::lock_guard lock(emitMutex);
      std::swap(queueCopy, eventQueue);
    }

    for (const auto& [type, event] : queueCopy) {
      std::lock_guard lock(handlerMap.mutex);
      auto it = handlerMap.handlers.find(type);
      if (it != handlerMap.handlers.end()) {
        for (auto& listener : it->second) {
          listener(event);
        }
      }
    }
  }

private:
  struct ThreadLocalHandlerMap {
    std::mutex mutex;
    std::unordered_map<std::type_index, std::vector<std::function<void(const EventVariant&)>>>
        handlers;
  };

  static auto getThreadLocalHandlers() -> ThreadLocalHandlerMap& {
    thread_local ThreadLocalHandlerMap handlerMap;
    return handlerMap;
  }

  std::mutex emitMutex;
  std::deque<std::pair<std::type_index, EventVariant>> eventQueue;
};

}
