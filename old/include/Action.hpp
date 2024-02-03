#pragma once

#include "ActionType.hpp"
#include "Vec2.hpp"

namespace Triton::Actions {

   using ActionTypes = std::variant<bool, float, Vec2>;

   class Action {
    public:
      Action(ActionType atype, ActionTypes newValue) : actionType(atype), value(newValue) {
      }

      Action() = delete;

      [[nodiscard]] ActionType getActionType() const {
         return actionType;
      }

      template <typename T>
      const T& readValue() const {
         static_assert(std::holds_alternative<T>(value), "Action Type mismatch");
         return std::get<T>(value);
      }

    private:
      ActionType actionType;
      ActionTypes value;
   };
}