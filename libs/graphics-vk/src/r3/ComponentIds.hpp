#pragma once

namespace tr {

enum class PassId : uint8_t {
  Culling = 0,
  Forward,
};

enum class ContextId : uint8_t {
  Culling = 0,
  Cube,
};

}
