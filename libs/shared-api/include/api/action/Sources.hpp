#pragma once

#include "api/action/Inputs.hpp"

namespace tr {

enum class SourceType : uint8_t {
  Boolean = 0,
  Float
};

struct Source {
  Source(MouseInput mouseInput, const SourceType sType) : src{mouseInput}, sourceType{sType} {
  }

  Source(Key key, const SourceType sType) : src{key}, sourceType{sType} {
  }

  Source(GamepadInput gpInput, const SourceType sType) : src{gpInput}, sourceType{sType} {
  }

  std::variant<MouseInput, Key, GamepadInput> src{};
  SourceType sourceType{};
};
}
