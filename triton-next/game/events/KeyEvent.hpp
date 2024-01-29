#pragma once

#include "Events.hpp"
#include "Key.hpp"

namespace Triton::Events {
   class KeyEvent : public Event {
    public:
      [[nodiscard]] Actions::Key getKey() const {
         return key;
      }

      EVENT_CLASS_CATEGORY(EventCategoryKeyboard | EventCategoryInput)

    protected:
      explicit KeyEvent(const Actions::Key key) : Event(), key(key) {
      }

    private:
      Actions::Key key;
   };

   class KeyPressedEvent final : public KeyEvent {
    public:
      explicit KeyPressedEvent(const Actions::Key key, bool const isRepeat = false) :
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

   class KeyReleasedEvent final : public KeyEvent {
    public:
      explicit KeyReleasedEvent(const Actions::Key key) : KeyEvent(key) {
      }

      [[nodiscard]] std::string toString() const override {
         std::stringstream ss;
         ss << "KeyReleasedEvent: " << static_cast<uint32_t>(getKey());
         return ss.str();
      }

      EVENT_CLASS_TYPE(KeyReleased)
   };

   class KeyTypedEvent final : public KeyEvent {
    public:
      explicit KeyTypedEvent(const Actions::Key key) : KeyEvent(key) {
      }

      [[nodiscard]] std::string toString() const override {
         std::stringstream ss;
         ss << "KeyTypedEvent: " << static_cast<uint32_t>(getKey());
         return ss.str();
      }

      EVENT_CLASS_TYPE(KeyTyped)
   };
}
