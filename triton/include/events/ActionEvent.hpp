#pragma once

#include "ActionType.hpp"
#include "Events.hpp"
#include "Key.hpp"

namespace Triton::Events {
   class ActionEvent final : public Event {
    public:
      explicit ActionEvent(const Actions::ActionType actionType) : actionType(actionType) {
      }

      [[nodiscard]] Actions::ActionType getActionType() const {
         return actionType;
      }

      [[nodiscard]] std::string toString() const override {
         std::stringstream ss;
         ss << "ActionEvent: " << getString(actionType);
         return ss.str();
      }

      EVENT_CLASS_CATEGORY(EventCategoryAction)
      EVENT_CLASS_TYPE(Action)
    private:
      Actions::ActionType actionType;
   };
}