#include "actions/ActionSystem.hpp"

#include "actions/Action.hpp"
#include "cm/Inputs.hpp"
#include "actions/Sources.hpp"

namespace tr::gp {

   void ActionSystem::mapSource(Source source, StateType sType, ActionType aType) {
      std::visit(
          [&](auto&& arg) {
             using T = std::decay_t<decltype(arg)>;
             if constexpr (std::is_same_v<T, Key>) {
                keyActionMap.insert_or_assign(arg, Action{aType, sType});
             } else if constexpr (std::is_same_v<T, MouseInput>) {
                mouseActionMap.insert_or_assign(arg, Action{aType, sType});
             } else if constexpr (std::is_same_v<T, GamepadInput>) {
             }
          },
          source.src);
   }

   void ActionSystem::setMouseState(bool captured) {
      if (captured) {
         firstMouse = true;
      }
   }

   void ActionSystem::mouseButtonCallback([[maybe_unused]] int button,
                                          [[maybe_unused]] int action,
                                          [[maybe_unused]] int mods) {
   }

   void ActionSystem::cursorPosCallback(double xpos, double ypos) {
      const auto deltaX = static_cast<float>(prevX - xpos);
      const auto deltaY = static_cast<float>(prevY - ypos);

      prevX = xpos;
      prevY = ypos;

      if (firstMouse) {
         firstMouse = !firstMouse;
         return;
      }

      if (deltaX != 0) {
         const auto xit = mouseActionMap.find(MouseInput::MOVE_X);
         if (xit != mouseActionMap.end()) {
            actionDelegate(Action{xit->second.actionType, xit->second.stateType, deltaX});
         }
      }

      if (deltaY != 0) {
         const auto yit = mouseActionMap.find(MouseInput::MOVE_Y);
         if (yit != mouseActionMap.end()) {
            actionDelegate(Action{yit->second.actionType, yit->second.stateType, deltaY});
         }
      }
   }

   void ActionSystem::keyCallback(gp::Key key, gp::ButtonState state) {

      const auto sourceIt = keyActionMap.find(key);
      if (sourceIt == keyActionMap.end()) {
         return;
      }
      const auto& source = sourceIt->second;

      if (state == ButtonState::Pressed) {
         actionDelegate(Action{source.actionType, source.stateType, true});
      } else if (state == ButtonState::Released) {
         actionDelegate(Action{source.actionType, source.stateType, false});
      }
   }

}
