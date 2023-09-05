#pragma once

#include "Bimap.hpp"
#include "Key.hpp"

namespace Input {
   void initializeGlfwKeyMap(Bimap<Key, int>& keyMap);
}
