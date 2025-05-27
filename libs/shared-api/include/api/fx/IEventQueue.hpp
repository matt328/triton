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

  virtual void subscribe(std::type_index type,
                         std::function<void(const EventVariant&)> listener,
                         std::string channel) = 0;

  virtual void emit(std::type_index type, EventVariant event, std::string channel) = 0;

  /// Dispatch any handlers that might have been subscribed on the current thread.
  /// Must be called periodically from any thread on which events have been subscribed to.
  virtual void dispatchPending() = 0;

  template <typename T>
  void subscribe(std::function<void(const T&)> listener, std::string channel = "default") {
    subscribe(
        typeid(T),
        [listener](const EventVariant& event) {
          if (const T* typed = std::get_if<T>(&event)) {
            listener(*typed);
          }
        },
        channel);
  }

  template <typename T>
  void subscribeWithVariant(std::function<void(const EventVariant&)> listener,
                            std::string channel = "default") {
    subscribe(
        typeid(T),
        [listener](const EventVariant& event) {
          if (std::holds_alternative<T>(event)) {
            listener(event);
          }
        },
        channel);
  }

  template <typename T>
  void subscribe(std::function<void(const T&, const EventVariant&)> listener,
                 std::string channel = "default") {
    subscribe(
        typeid(T),
        [listener](const EventVariant& event) {
          if (const T* typed = std::get_if<T>(&event)) {
            listener(*typed, event);
          }
        },
        channel);
  }

  template <typename T>
  void emit(T event, std::string channel = "default") {
    emit(typeid(T), EventVariant(std::move(event)), std::move(channel));
  }
};

}
