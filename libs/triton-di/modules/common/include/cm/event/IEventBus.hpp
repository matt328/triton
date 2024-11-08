#pragma once

#include "cm/Inputs.hpp"
#include "Actions.hpp"

namespace tr::cm::evt {

   struct WindowIconified {
      int iconified;
   };

   struct WindowClosed {};

   struct Key {
      cm::Key key;
      cm::ButtonState buttonState;
   };

   struct MouseMoved {
      double x;
      double y;
   };

   struct Fullscreen {
      bool isFullscreen;
   };

   struct MouseCaptured {
      bool isMouseCaptured;
   };

   struct MouseButton {
      int button;
      int action;
      int mods;
   };

   struct PlayerMoved {
      int playerId;
      float x, y;
   };

   struct PlayerScored {
      int playerId;
      int score;
   };

   using EventVariant = std::variant<WindowIconified,
                                     WindowClosed,
                                     Key,
                                     MouseMoved,
                                     MouseButton,
                                     Fullscreen,
                                     MouseCaptured,
                                     Action,
                                     PlayerMoved,
                                     PlayerScored>;

   class IEventBus {
    public:
      virtual ~IEventBus() = default;

      // Non-templated methods for subscribing and emitting events
      virtual void subscribe(std::type_index type,
                             std::function<void(const EventVariant&)> listener) = 0;
      virtual void emit(std::type_index type, const EventVariant& event) = 0;

      // Templated helper methods for type-safe subscribe and emit
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