#pragma once

#include "Events.hpp"

namespace Triton::Events {

   class WindowCloseEvent : public Event {
    public:
      WindowCloseEvent() = default;

      EVENT_CLASS_TYPE(WindowClose)
      EVENT_CLASS_CATEGORY(EventCategoryApplication)
   };

   class FixedUpdateEvent : public Event {
    public:
      FixedUpdateEvent() = default;

      EVENT_CLASS_TYPE(FixedUpdate)
      EVENT_CLASS_CATEGORY(EventCategoryApplication)
   };

   class UpdateEvent : public Event {
    public:
      UpdateEvent(double blendFactor) : blendFactor(blendFactor) {
      }

      [[nodiscard]] double getBlendFactor() const {
         return blendFactor;
      }

      EVENT_CLASS_TYPE(Update)
      EVENT_CLASS_CATEGORY(EventCategoryApplication)

    private:
      double blendFactor;
   };

   class RenderEvent : public Event {
    public:
      RenderEvent() = default;

      EVENT_CLASS_TYPE(Render)
      EVENT_CLASS_CATEGORY(EventCategoryApplication)
   };

   class ShutdownEvent : public Event {
    public:
      ShutdownEvent() = default;

      EVENT_CLASS_TYPE(Shutdown)
      EVENT_CLASS_CATEGORY(EventCategoryApplication)
   };

};