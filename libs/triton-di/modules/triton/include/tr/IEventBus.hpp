#pragma once

#include "Events.hpp"

namespace tr {

   class IEventBus {
    public:
      virtual ~IEventBus() = default;

      virtual void subscribe(std::type_index type,
                             std::function<void(const EventVariant&)> listener) = 0;

      virtual void emit(std::type_index type, const EventVariant& event) = 0;

      template <typename T>
      void subscribe(std::function<void(const T&)> listener) {
         subscribe(typeid(T), [listener](const EventVariant& event) {
            if (const T* typedEvent = std::get_if<T>(&event)) {
               listener(*typedEvent);
            }
         });
      }

      template <typename T>
      void emit(const T& event) {
         emit(typeid(T), EventVariant(event));
      }
   };
}
