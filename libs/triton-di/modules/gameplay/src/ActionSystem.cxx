#include "gp/action/ActionSystem.hpp"
#include "tr/IEventBus.hpp"

namespace tr::gp {

   ActionSystem::ActionSystem(const std::shared_ptr<tr::IEventBus>& eventBus) {

      Log.debug("Creating ActionSystem");

      eventBus->subscribe<tr::MouseCaptured>([&](const tr::MouseCaptured& event) {
         Log.debug("Mouse Capture, captured: {0}", event.isMouseCaptured);
         if (event.isMouseCaptured) {
            firstMouse = true;
         }
      });

      eventBus->subscribe<tr::MouseMoved>([&](const tr::MouseMoved& event) {
         Log.debug("Mouse Move, position: ({0}, {1})", event.x, event.y);
         const auto deltaX = static_cast<float>(prevX - event.x);
         const auto deltaY = static_cast<float>(prevY - event.y);

         prevX = event.x;
         prevY = event.y;

         if (firstMouse) {
            firstMouse = !firstMouse;
            return;
         }

         if (deltaX != 0) {
            if (const auto xit = mouseActionMap.find(cm::MouseInput::MOVE_X);
                xit != mouseActionMap.end()) {
               eventBus->emit(tr::Action{xit->second.actionType, xit->second.stateType, deltaX});
            }
         }

         if (deltaY != 0) {
            if (const auto yit = mouseActionMap.find(cm::MouseInput::MOVE_Y);
                yit != mouseActionMap.end()) {
               eventBus->emit(tr::Action{yit->second.actionType, yit->second.stateType, deltaY});
            }
         }
      });
   }

   ActionSystem::~ActionSystem() {
      keyActionMap.clear();
      mouseActionMap.clear();
   }

   void ActionSystem::mapSource(Source source, tr::StateType sType, tr::ActionType aType) {
      std::visit(
          [&]<typename T>(T&& arg) {
             using U = std::decay_t<T>;
             if constexpr (std::is_same_v<U, tr::KeyEvent>) {
                keyActionMap.insert_or_assign(arg, tr::Action{aType, sType});
             } else if constexpr (std::is_same_v<U, tr::MouseInput>) {
                mouseActionMap.insert_or_assign(arg, tr::Action{aType, sType});
             } else if constexpr (std::is_same_v<U, tr::GamepadInput>) {
             }
          },
          source.src);
   }

}