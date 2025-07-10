#pragma once

#include <utility>

#include "api/fx/Events.hpp"

namespace tr {

class IEventQueue {
public:
  IEventQueue() = default;
  virtual ~IEventQueue() = default;

  IEventQueue(const IEventQueue&) = default;
  IEventQueue(IEventQueue&&) = delete;
  auto operator=(const IEventQueue&) -> IEventQueue& = default;
  auto operator=(IEventQueue&&) -> IEventQueue& = delete;

  // Type-erased emit
  virtual void emitErased(std::type_index type,
                          std::shared_ptr<void> payload,
                          const std::string& channel) = 0;

  // Type-erased subscribe
  virtual void subscribeErased(std::type_index type,
                               std::function<void(std::shared_ptr<void>)> listener,
                               const std::string& channel) = 0;

  // Dispatch queued events on current thread
  virtual void dispatchPending() = 0;

  // Convenience templated API
  template <typename T>
  void emit(T event, const std::string& channel = "default") {
    emitErased(typeid(T), std::make_shared<T>(std::move(event)), channel);
  }

  template <typename T>
  void subscribe(std::function<void(const std::shared_ptr<T>&)> listener,
                 const std::string& channel = "default") {
    subscribeErased(
        typeid(T),
        [listener](std::shared_ptr<void> ptr) { listener(std::static_pointer_cast<T>(ptr)); },
        channel);
  }
};

}
