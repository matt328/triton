#pragma once

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

  virtual void subscribe(std::type_index type,
                         std::function<void(const EventVariant&)> listener) = 0;

  virtual void emit(std::type_index type, const EventVariant& event) = 0;

  /// Dispatch any handlers that might have been subscribed on the current thread.
  /// Must be called periodically from any thread on which events have been subscribed to.
  virtual void dispatchPending() = 0;

  template <typename T>
  void subscribe(std::function<void(const T&)> listener) {
    subscribe(typeid(T), [listener](const EventVariant& event) {
      if (const T* typed = std::get_if<T>(&event)) {
        listener(*typed);
      }
    });
  }

  template <typename T>
  void emit(const T& event) {
    emit(typeid(T), EventVariant(event));
  }
};

}
