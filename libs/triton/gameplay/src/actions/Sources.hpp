#pragma once

#include "cm/Inputs.hpp"

namespace tr::gp {
   enum class SourceType : uint32_t {
      Boolean = 0,
      Float
   };

   struct Source {
      Source(cm::MouseInput me, SourceType sType) : src{me}, sourceType{sType} {
      }

      Source(cm::Key key, SourceType sType) : src{key}, sourceType{sType} {
      }

      Source(cm::GamepadInput in, SourceType sType) : src{in}, sourceType{sType} {
      }

      std::variant<cm::MouseInput, cm::Key, cm::GamepadInput> src;
      SourceType sourceType;
   };
}
