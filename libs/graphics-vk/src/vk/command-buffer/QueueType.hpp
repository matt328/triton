#pragma once

namespace tr {
enum class QueueType : uint8_t {
  Compute = 0,
  Graphics,
  Transfer
};
}
