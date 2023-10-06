#pragma once

#include "Events.hpp"
#include "Keys.hpp"

namespace Events {
   class KeyEvent : public Event {
    public:
      [[nodiscard]] Core::Key getKey() const {
         return key;
      }

      EVENT_CLASS_CATEGORY(EventCategoryKeyboard | EventCategoryInput)

    protected:
      KeyEvent(const Core::Key key) : Event(), key(key) {
      }

    private:
      Core::Key key;
   };

   class KeyPressedEvent : public KeyEvent {
    public:
      KeyPressedEvent(const Core::Key key, bool isRepeat = false) :
          KeyEvent(key), isRepeated(isRepeat) {
      }

      [[nodiscard]] bool isRepeat() const {
         return isRepeated;
      }

      EVENT_CLASS_TYPE(KeyPressed)
    private:
      bool isRepeated;
   };
}
