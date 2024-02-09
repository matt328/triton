#include "ActionSet.hpp"
#include "actions/Gamepad.hpp"

namespace Triton::Actions {

   ActionType ActionSet::getActionForKey(Key key) const {
      const auto it = keyMap.find(key);
      assert(it != keyMap.end());
      return it->second;
   }

   ActionType ActionSet::getActionForMouse(MouseInput mouse) const {
      const auto it = mouseMap.find(mouse);
      assert(it != mouseMap.end());
      return it->second;
   }

   ActionType ActionSet::getActionForGamepad(GamepadInput gp) const {
      const auto it = gamepadMap.find(gp);
      assert(it != gamepadMap.end());
      return it->second;
   }

   ActionType ActionSet::getActionForSourceEvent(const SourceEvent source) const {
      if (const auto mouseInput = std::get_if<MouseInput>(&source.src)) {
         return getActionForMouse(*mouseInput);
      } else if (const auto key = std::get_if<Key>(&source.src)) {
         return getActionForKey(*key);
      } else if (const auto gp = std::get_if<GamepadInput>(&source.src)) {
         return getActionForGamepad(*gp);
      } else {
         throw std::runtime_error("Unhandled Input Type");
      }
   }

   void ActionSet::bindSource(Source source, ActionType actionType) {
      if (const auto mouseInput = std::get_if<MouseInput>(&source.src)) {
         mouseMap.insert(std::make_pair(*mouseInput, actionType));
      } else if (const auto key = std::get_if<Key>(&source.src)) {
         keyMap.insert(std::make_pair(*key, actionType));
      } else if (const auto gp = std::get_if<GamepadInput>(&source.src)) {
         gamepadMap.insert(std::make_pair(*gp, actionType));
      } else {
         throw std::runtime_error("Unhandled Input Type");
      }
   }
}