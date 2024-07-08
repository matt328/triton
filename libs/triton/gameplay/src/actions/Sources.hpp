#pragma once

#include "cm/Inputs.hpp"

namespace tr::gp {
   enum class SourceType : uint32_t {
      Boolean = 0,
      Float
   };

   struct Source {
      Source(MouseInput me, SourceType sType) : src{me}, sourceType{sType} {
      }

      Source(Key key, SourceType sType) : src{key}, sourceType{sType} {
      }

      Source(GamepadInput in, SourceType sType) : src{in}, sourceType{sType} {
      }

      std::variant<MouseInput, Key, GamepadInput> src;
      SourceType sourceType;
   };
}