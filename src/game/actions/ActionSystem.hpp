#pragma once

#include "ActionState.hpp"
#include "ActionSet.hpp"

namespace Triton::Actions {
   enum class ActionSets : uint32_t {
      Main = 0,
      Menu
   };

   class ActionSystem {
    public:
      ActionSystem(GLFWwindow& window) : window(window) {
      }
      ~ActionSystem() = default;

      ActionSystem(const ActionSystem&) = default;
      ActionSystem(ActionSystem&&) = default;
      ActionSystem& operator=(const ActionSystem&) = delete;
      ActionSystem& operator=(ActionSystem&&) = delete;

      ActionSet& createActionSet(ActionSets name);
      void setActiveSet(ActionSets newActiveSet);

      [[nodiscard]] ActionState& getActionState() {
         return actionState;
      }

      void update();

    private:
      GLFWwindow& window;
      std::unordered_map<ActionSets, ActionSet> actionSetMap;
      ActionSets activeSet{};
      ActionState actionState{};

      [[nodiscard]] bool sourceToBool(const Source source) const;
   };
}