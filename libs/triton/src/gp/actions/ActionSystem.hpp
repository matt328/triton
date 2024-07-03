#pragma once

#include "gp/EntitySystem.hpp"
namespace tr::gp {
   enum class ActionSets : uint32_t {
      Main = 0,
      Menu
   };

   struct Action;
   struct Source;
   enum class StateType : uint32_t;
   enum class ActionType : uint32_t;
   enum class Key : uint32_t;
   enum class MouseInput;

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