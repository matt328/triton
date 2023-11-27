#pragma once

#include "ActionType.hpp"
#include "Events.hpp"
#include "Key.hpp"

namespace Triton::Events {
   class ActionEvent : public Event {
    public:
      ActionEvent(const Actions::ActionType actionType) : actionType(actionType) {
      }

      [[nodiscard]] Actions::ActionType getActionType() const {
         return actionType;
      }

      EVENT_CLASS_CATEGORY(EventCategoryAction)
      EVENT_CLASS_TYPE(Action)
    private:
      Actions::ActionType actionType;
   };
}