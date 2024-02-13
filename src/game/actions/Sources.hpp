#pragma once

#include "Key.hpp"
#include "Mouse.hpp"
#include "Gamepad.hpp"

namespace Triton::Actions {
   enum class SourceType : uint32_t {
      Boolean = 0,
      Float
   };

   struct Source {
      Source(MouseInput me) : src{me} {
      }

      Source(Key key) : src(key) {
      }

      Source(GamepadInput in) : src(in) {
      }

      std::variant<MouseInput, Key, GamepadInput> src;
   };
}