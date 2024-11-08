#include "action/ActionSystem.hpp"
#include "event/IEventBus.hpp"

namespace tr::gp {

   namespace evt = cm::evt;

   ActionSystem::ActionSystem(const std::shared_ptr<cm::evt::IEventBus>& eventBus) {

      Log.debug("Creating ActionSystem");

      eventBus->subscribe<cm::evt::MouseCaptured>([&](const cm::evt::MouseCaptured& event) {
         Log.debug("Mouse Capture, captured: {0}", event.isMouseCaptured);
         if (event.isMouseCaptured) {
            firstMouse = true;
         }
      });

      eventBus->subscribe<cm::evt::MouseMoved>([&](const cm::evt::MouseMoved& event) {
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
               eventBus->emit(evt::Action{xit->second.actionType, xit->second.stateType, deltaX});
            }
         }

         if (deltaY != 0) {
            if (const auto yit = mouseActionMap.find(cm::MouseInput::MOVE_Y);
                yit != mouseActionMap.end()) {
               eventBus->emit(evt::Action{yit->second.actionType, yit->second.stateType, deltaY});
            }
         }
      });
   }

   ActionSystem::~ActionSystem() {
      keyActionMap.clear();
      mouseActionMap.clear();
   }

   void ActionSystem::mapSource(Source source, evt::StateType sType, evt::ActionType aType) {
      std::visit(
          [&]<typename T>(T&& arg) {
             using U = std::decay_t<T>;
             if constexpr (std::is_same_v<U, cm::Key>) {
                keyActionMap.insert_or_assign(arg, evt::Action{aType, sType});
             } else if constexpr (std::is_same_v<U, cm::MouseInput>) {
                mouseActionMap.insert_or_assign(arg, evt::Action{aType, sType});
             } else if constexpr (std::is_same_v<U, cm::GamepadInput>) {
             }
          },
          source.src);
   }

}