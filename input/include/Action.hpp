#pragma once

#include "ActionType.hpp"
#include "Vec2.hpp"

namespace Input {

   using ActionTypes = std::variant<bool, float, Vec2>;

   class Action {
    public:
      Action(ActionType atype, ActionTypes newValue) : actionType(atype), value(newValue) {
      }

      Action() = delete;

      [[nodiscard]] ActionType getActionType() const {
         return actionType;
      }

      [[nodiscard]] const Vec2& getVec2Value() const {
         assert(std::holds_alternative<glm::vec2>(value));
         return std::get<Vec2>(value);
      }

      [[nodiscard]] const float& getFloatValue() const {
         assert(std::holds_alternative<float>(value));
         return std::get<float>(value);
      }

      [[nodiscard]] const bool& getBooleanValue() const {
         assert(std::holds_alternative<bool>(value));
         return std::get<bool>(value);
      }

      template <typename T>
      const T& getValue() const {
         static_assert(std::holds_alternative<T>(value), "Action Type mismatch");
         return std::get<T>(value);
      }

    private:
      ActionType actionType;
      ActionTypes value;
   };
}