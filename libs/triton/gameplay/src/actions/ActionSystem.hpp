#pragma once

#include "cm/Inputs.hpp"

namespace tr::gp {
   enum class ActionSets : uint32_t {
      Main = 0,
      Menu
   };

   struct Action;
   struct Source;
   enum class StateType : uint32_t;
   enum class ActionType : uint32_t;

   class ActionSystem {
    public:
      ActionSystem() = default;
      ~ActionSystem() = default;

      ActionSystem(const ActionSystem&) = default;
      ActionSystem(ActionSystem&&) noexcept = default;
      ActionSystem& operator=(const ActionSystem&) = delete;
      ActionSystem& operator=(ActionSystem&&) = delete;

      [[nodiscard]] auto& getDelegate() {
         return actionDelegate;
      }

      void mapSource(Source source, StateType sType, ActionType aType);

      void mapKeyState(cm::Key key, ActionType aType);
      void mapKeyAction(cm::Key, ActionType aType);

      void keyCallback(cm::Key key, cm::ButtonState state);
      void cursorPosCallback(double xpos, double ypos);
      void mouseButtonCallback(int button, int action, int mods);
      void setMouseState(bool captured);

    private:
      entt::delegate<void(Action)> actionDelegate{};

      double prevX{}, prevY{};
      bool firstMouse = true;
      std::unordered_map<cm::Key, Action> keyActionMap{};
      std::unordered_map<cm::MouseInput, Action> mouseActionMap{};
   };
}