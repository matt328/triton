#pragma once

namespace Triton::Actions {
   enum class GamepadInput : uint32_t {
      None = 0,
      Button_X,
      Button_Y,
      Button_A,
      Button_B,
      LeftJoystickAxisX,
      LeftJoystickAxisY
      // TODO
   };
}