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

      [[nodiscard]] std::string toString() const override {
         std::stringstream ss;
         ss << "KeyPressedEvent: " << static_cast<uint32_t>(getKey()) << " (repeat = " << isRepeat()
            << ")";
         return ss.str();
      }

      EVENT_CLASS_TYPE(KeyPressed) private : bool isRepeated;
   };

   class KeyReleasedEvent : public KeyEvent {
    public:
      KeyReleasedEvent(const Core::Key key) : KeyEvent(key) {
      }

      [[nodiscard]] std::string toString() const override {
         std::stringstream ss;
         ss << "KeyReleasedEvent: " << static_cast<uint32_t>(getKey());
         return ss.str();
      }

      EVENT_CLASS_TYPE(KeyReleased)
   };

   class KeyTypedEvent : public KeyEvent {
    public:
      KeyTypedEvent(const Core::Key key) : KeyEvent(key) {
      }

      [[nodiscard]] std::string toString() const override {
         std::stringstream ss;
         ss << "KeyTypedEvent: " << static_cast<uint32_t>(getKey());
         return ss.str();
      }

      EVENT_CLASS_TYPE(KeyTyped)
   };
}
