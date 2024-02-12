#include "ActionSet.hpp"
#include "Gamepad.hpp"

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
      Key* key = std::get_if<Key>(&source.src);
      if (key != nullptr) {
         Log::info << "inserting key: " << key << std::endl;
         keyMap.insert({*key, actionType});
      }
   }
}