#pragma once

#include "Key.hpp"
#include "Mouse.hpp"
#include "Gamepad.hpp"

namespace Triton::Actions {
   struct Source {
      Source(MouseInput me) : src{me} {
      }

      Source(Key key) : src(key) {
      }

      Source(GamepadInput in) : src(in) {
      }

      std::variant<MouseInput, Key, GamepadInput> src;
   };

   struct SourceEvent : public Source {

      SourceEvent(const Source s) : Source{s} {
      }

      SourceEvent(const Source s, float value) : Source{s}, value{value} {
      }
      std::optional<float> value{};
   };
}