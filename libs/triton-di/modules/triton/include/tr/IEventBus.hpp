#pragma once

#include "Events.hpp"

namespace tr {

class IEventBus {
public:
  IEventBus() = default;
  virtual ~IEventBus() = default;

  IEventBus(const IEventBus&) = default;
  IEventBus(IEventBus&&) = delete;
  auto operator=(const IEventBus&) -> IEventBus& = default;
  auto operator=(IEventBus&&) -> IEventBus& = delete;

  virtual void subscribe(std::type_index type,
                         std::function<void(const EventVariant&)> listener) = 0;

  virtual void emit(std::type_index type, const EventVariant& event) = 0;

  /// Example: `eventBus->subscribe<EventType>([](const EventType& event) {})`
  template <typename T>
  void subscribe(std::function<void(const T&)> listener) {
    subscribe(typeid(T), [listener](const EventVariant& event) {
      if (const T* typedEvent = std::get_if<T>(&event)) {
        listener(*typedEvent);
      }
    });
  }

  /// Example: `eventBus->emit(EventType{})`
  /// Don't forget to register EventType with the variant in `Events.hpp`
  template <typename T>
  void emit(const T& event) {
    emit(typeid(T), EventVariant(event));
  }
};
}
