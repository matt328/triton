#pragma once

#include "ActionType.hpp"

namespace Triton::Actions {

   using BoolMap = std::unordered_multimap<ActionType, bool>;
   using FloatMap = std::unordered_multimap<ActionType, float>;

   class ActionState {
    public:
      ActionState() = default;

      void nextFrame();

      void setBool(ActionType actionType, bool value);
      void setFloat(ActionType actionType, float value);

      [[nodiscard]] bool getBool(ActionType actionType) const;
      [[nodiscard]] bool getBoolWasDown(ActionType actionType) const;

      [[nodiscard]] float getFloat(ActionType actionType) const;
      [[nodiscard]] float getFloatDelta(ActionType actionType) const;

    private:
      BoolMap currentBoolMap{};
      BoolMap previousBoolMap{};

      FloatMap currentFloatMap{};
      FloatMap previousFloatMap{};
   };
}
