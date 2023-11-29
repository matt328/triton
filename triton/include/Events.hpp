#pragma once

namespace Triton::Events {

#define BIT(x) (1 << x)

   enum class EventType {
      None = 0,
      WindowClose,
      WindowResize,
      WindowFocus,
      WindowLostFocus,
      WindowMoved,
      FixedUpdate,
      Update,
      Render,
      Shutdown,
      KeyPressed,
      KeyReleased,
      KeyTyped,
      MouseButtonPressed,
      MouseButtonReleased,
      MouseMoved,
      MouseScrolled,
      Action
   };

   enum EventCategory {
      None = 0,
      EventCategoryApplication = BIT(0),
      EventCategoryInput = BIT(1),
      EventCategoryKeyboard = BIT(2),
      EventCategoryMouse = BIT(3),
      EventCategoryMouseButton = BIT(4),
      EventCategoryAction = BIT(5)
   };

#define EVENT_CLASS_TYPE(type)                                                                     \
   static EventType getStaticType() {                                                              \
      return EventType::type;                                                                      \
   }                                                                                               \
   virtual EventType getEventType() const override {                                               \
      return getStaticType();                                                                      \
   }                                                                                               \
   virtual const char* getName() const override {                                                  \
      return #type;                                                                                \
   }

#define EVENT_CLASS_CATEGORY(category)                                                             \
   virtual int getCategoryFlags() const override {                                                 \
      return category;                                                                             \
   }

   class Event {
    public:
      Event() = default;
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

      [[nodiscard]] bool isInCategory(const EventCategory category) const {
         return getCategoryFlags() & category;
      }
   };

   class EventDispatcher {
    public:
      explicit EventDispatcher(Event& event) : event(event) {
      }

      template <typename T, typename F>
      bool dispatch(const F& fn) {
         if (event.getEventType() == T::getStaticType() && !event.handled) {
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