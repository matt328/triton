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
      Source(MouseInput me, SourceType sType) : src{me}, sourceType{sType} {
      }

      Source(Key key, SourceType sType) : src{key}, sourceType{sType} {
      }

      Source(GamepadInput in) : src{in} {
      }

      std::variant<MouseInput, Key, GamepadInput> src;
      SourceType sourceType;
   };
}