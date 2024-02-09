#pragma once

namespace Triton::Actions {
   enum class ActionSets : uint32_t {
      Main = 0,
      Menu
   };

   class ActionSet;

   class ActionSystem {
    public:
      ActionSystem(GLFWwindow& window) : window(window) {
      }
      ~ActionSystem() = default;

      ActionSystem(const ActionSystem&) = default;
      ActionSystem(ActionSystem&&) = delete;
      ActionSystem& operator=(const ActionSystem&) = delete;
      ActionSystem& operator=(ActionSystem&&) = delete;

      ActionSet& createActionSet(ActionSets name);
      void setActiveSet(ActionSets newActiveSet);

      void update();

    private:
      GLFWwindow& window;
      std::unordered_map<ActionSets, ActionSet> actionSetMap;
      ActionSets activeSet{};
   };
}