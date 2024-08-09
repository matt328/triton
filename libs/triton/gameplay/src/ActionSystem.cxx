#include "actions/ActionSystem.hpp"

#include "actions/Action.hpp"
#include "cm/Inputs.hpp"
#include "actions/Sources.hpp"

namespace tr::gp {

   void ActionSystem::mapSource(Source source, const StateType sType, const ActionType aType) {
      std::visit(
          [&]<typename T>(T&& arg) {
             using U = std::decay_t<T>;
             if constexpr (std::is_same_v<U, cm::Key>) {
                keyActionMap.insert_or_assign(arg, Action{aType, sType});
             } else if constexpr (std::is_same_v<U, cm::MouseInput>) {
                mouseActionMap.insert_or_assign(arg, Action{aType, sType});
             } else if constexpr (std::is_same_v<U, cm::GamepadInput>) {
             }
          },
          source.src);
   }

   void ActionSystem::setMouseState(const bool captured) {
      if (captured) {
         firstMouse = true;
      }
   }

   void ActionSystem::mouseButtonCallback([[maybe_unused]] int button,
                                          [[maybe_unused]] int action,
                                          [[maybe_unused]] int mods) {
   }

   void ActionSystem::cursorPosCallback(const double xpos, const double ypos) {
      const auto deltaX = static_cast<float>(prevX - xpos);
      const auto deltaY = static_cast<float>(prevY - ypos);

      prevX = xpos;
      prevY = ypos;

      if (firstMouse) {
         firstMouse = !firstMouse;
         return;
      }

      if (deltaX != 0) {
         if (const auto xit = mouseActionMap.find(cm::MouseInput::MOVE_X);
             xit != mouseActionMap.end()) {
            actionDelegate(Action{xit->second.actionType, xit->second.stateType, deltaX});
         }
      }

      if (deltaY != 0) {
         if (const auto yit = mouseActionMap.find(cm::MouseInput::MOVE_Y);
             yit != mouseActionMap.end()) {
            actionDelegate(Action{yit->second.actionType, yit->second.stateType, deltaY});
         }
      }
   }

   void ActionSystem::keyCallback(const cm::Key key, const cm::ButtonState state) {

      const auto sourceIt = keyActionMap.find(key);
      if (sourceIt == keyActionMap.end()) {
         return;
      }
      const auto& [actionType, stateType, value] = sourceIt->second;

      if (state == cm::ButtonState::Pressed) {
         actionDelegate(Action{actionType, stateType, true});
      } else if (state == cm::ButtonState::Released) {
         actionDelegate(Action{actionType, stateType, false});
      }
   }

}
