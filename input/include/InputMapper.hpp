#pragma once
#include "Key.hpp"
#include "KeyState.hpp"
#include "Bimap.hpp"
#include "GlfwKeyMap.hpp"
#include "ActionManager.hpp"

namespace Input {
   class InputMapper {
    public:
      InputMapper(ActionManager& actionMapper) : actionMapper(actionMapper) {
         initializeGlfwKeyMap(keyMap);
      }
      void keyCallback(const int key,
                       [[maybe_unused]] int scancode,
                       const int action,
                       [[maybe_unused]] int mods) {

         const auto mappedKey = keyMap.getKey(key);
         if (action == GLFW_PRESS) {
            keyStateMap[mappedKey] = KeyState::Down;
            actionMapper.keyPressed(mappedKey);
         } else if (action == GLFW_RELEASE) {
            keyStateMap[mappedKey] = KeyState::Up;
            actionMapper.keyReleased(mappedKey);
         }
      }

    private:
      std::unordered_map<Key, KeyState> keyStateMap;
      Bimap<Key, int> keyMap;
      ActionManager& actionMapper;
   };
}
