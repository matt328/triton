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

   class EventBus {
    public:
      EventBus() = default;

      template <typename T>
      void subscribe(std::function<void(const T&)> listener) {
         auto& listeners = listenersMap[typeid(T)];
         listeners.push_back([listener](const EventVariant& event) {
            if (const T* typedEvent = std::get_if<T>(&event)) {
               listener(*typedEvent);
            }
         });
      }

      template <typename T>
      void emit(const T& event) {
         auto it = listenersMap.find(typeid(T));
         if (it != listenersMap.end()) {
            for (auto& listener : it->second) {
               listener(event);
            }
         }
      }

    private:
      std::unordered_map<std::type_index, std::vector<std::function<void(const EventVariant&)>>>
          listenersMap;
   };
}