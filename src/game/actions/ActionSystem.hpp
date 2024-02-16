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
      ActionSystem() = default;
      ~ActionSystem() = default;

      ActionSystem(const ActionSystem&) = default;
      ActionSystem(ActionSystem&&) noexcept = default;
      ActionSystem& operator=(const ActionSystem&) = delete;
      ActionSystem& operator=(ActionSystem&&) = delete;

      [[nodiscard]] entt::delegate<void(Action)>& getDelegate() {
         return actionDelegate;
      }

      void mapSource(Source source, StateType sType, ActionType aType);

      void mapKeyState(Key key, ActionType aType);
      void mapKeyAction(Key, ActionType aType);

      void keyCallback(int key, int scancode, int action, int mods);
      void cursorPosCallback(double xpos, double ypos);
      void mouseButtonCallback(int button, int action, int mods);
      void setMouseState(bool captured);

    private:
      entt::delegate<void(Action)> actionDelegate{};

      double prevX{}, prevY{};
      bool firstMouse = true;
      std::unordered_map<Key, Action> keyActionMap{};
      std::unordered_map<MouseInput, Action> mouseActionMap{};
   };
}