#pragma once

#include "ActionState.hpp"
#include "ActionSet.hpp"
#include "Action.hpp"

namespace Triton::Actions {
   enum class ActionSets : uint32_t {
      Main = 0,
      Menu
   };

   class ActionSystem {
    public:
      ActionSystem(GLFWwindow& window);
      ~ActionSystem() = default;

      ActionSystem(const ActionSystem&) = default;
      ActionSystem(ActionSystem&&) = default;
      ActionSystem& operator=(const ActionSystem&) = delete;
      ActionSystem& operator=(ActionSystem&&) = delete;

      ActionSet& createActionSet(ActionSets name);
      void setActiveSet(ActionSets newActiveSet);

      [[nodiscard]] entt::delegate<void(Action)>& getDelegate() {
         return actionDelegate;
      }

      void keyCallback(int key, int scancode, int action, int mods);
      void cursorPosCallback(double xpos, double ypos);
      void mouseButtonCallback(int button, int action, int mods);
      void setMouseState(bool captured);

    private:
      GLFWwindow& window;
      std::unordered_map<ActionSets, ActionSet> actionSetMap;
      ActionSets activeSet{};

      entt::delegate<void(Action)> actionDelegate{};

      double prevX{}, prevY{};
      bool firstMouse = true;
   };
}