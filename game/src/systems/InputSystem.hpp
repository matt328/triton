#pragma once

namespace Actions {
   enum Action {
      MoveForward = 0,
      MoveBackward,
      StrafeLeft,
      StrafeRight
   };
   [[maybe_unused]] static std::string toString(const Action actionId) {
      switch (actionId) {
         case Action::MoveForward:
            return "Move Forward";
         case Action::MoveBackward:
            return "Move Backward";
         case Action::StrafeLeft:
            return "Strafe Left";
         case Action::StrafeRight:
            return "Strafe Right";
         default:
            return "Unknown Action ID";
      }
   };
}

using ActionDelegate = entt::delegate<void(const Actions::Action&)>;
// TODO: Remove GLFW dependency from game lib
class InputSystem {
 public:
   InputSystem() : actionMap({}) {
      actionMap.insert({{GLFW_KEY_W, Actions::Action::MoveForward},
                        {GLFW_KEY_S, Actions::Action::MoveBackward},
                        {GLFW_KEY_A, Actions::Action::StrafeLeft},
                        {GLFW_KEY_D, Actions::Action::StrafeRight}});
      actionDelegate = ActionDelegate{};
   };
   InputSystem(const InputSystem&) = default;
   InputSystem(InputSystem&&) = delete;
   InputSystem& operator=(const InputSystem&) = default;
   InputSystem& operator=(InputSystem&&) = delete;
   ~InputSystem() = default;

   void keyCallback(const int key,
                    [[maybe_unused]] int scancode,
                    const int action,
                    [[maybe_unused]] int mods) const {
      if (action == GLFW_PRESS && actionMap.find(key) != actionMap.end()) {
         // Change Action to a struct so it has a payload of which entity it should go to
         // Consult some map to determine which entities are responding to which Actions
         actionDelegate(actionMap.at(key));
      }
   }

   ActionDelegate& getActionDelegate() {
      return actionDelegate;
   }

 private:
   std::unordered_map<int, const Actions::Action> actionMap = {};
   ActionDelegate actionDelegate;
};