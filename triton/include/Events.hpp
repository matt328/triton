#pragma once

#define BIT(x) (1 << x)

namespace Events {
   enum class EventType {
      None = 0,
      WindowClose,
      WindowResize,
      WindowFocus,
      WindowLostFocus,
      WindowMoved,
      AppTick,
      AppUpdate,
      AppRender,
      KeyPressed,
      KeyReleased,
      KeyTyped,
      MouseButtonPressed,
      MouseButtonReleased,
      MouseMoved,
      MouseScrolled
   };

   enum EventCategory {
      None = 0,
      EventCategoryApplication = BIT(0),
      EventCategoryInput = BIT(1),
      EventCategoryKeyboard = BIT(2),
      EventCategoryMouse = BIT(3),
      EventCategoryMouseButton = BIT(4)
   };

#define EVENT_CLASS_TYPE(type)                                                                     \
   static EventType GetStaticType() { return EventType::type; }                                    \
   virtual EventType GetEventType() const override { return GetStaticType(); }                     \
   virtual const char* GetName() const override { return #type; }

#define EVENT_CLASS_CATEGORY(category)                                                             \
   virtual int GetCategoryFlags() const override { return category; }

   class Event {
    public:
      Event(const Event&) = default;
      Event(Event&&) = delete;

      Event& operator=(const Event&) = default;
      Event& operator=(Event&&) = delete;

      virtual ~Event() = default;

      bool handled = false;

      [[nodiscard]] virtual EventType getEventType() const = 0;
      [[nodiscard]] virtual const char* getName() const = 0;
      [[nodiscard]] virtual int getCategoryFlags() const = 0;
      [[nodiscard]] virtual std::string toString() const {
         return getName();
      }

      bool isInCategory(EventCategory category) {
         return getCategoryFlags() & category;
      }
   };

   class EventDispatcher {
    public:
      EventDispatcher(Event& event) : event(event) {
      }

      template <typename T, typename F>
      bool dispatch(const F& fn) {
         if (event.getEventType() == T::getStaticType()) {
            event.handled |= fn(static_cast<T&>(event));
            return true;
         }
         return false;
      }

    private:
      Event& event;
   };

   inline std::ostream& operator<<(std::ostream& os, const Event& e) {
      return os << e.toString();
   }
}