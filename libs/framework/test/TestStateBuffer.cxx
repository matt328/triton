#include <catch2/catch_test_macros.hpp>

#include "fx/QueueStateBuffer.hpp"
#include "api/gfx/SimState.hpp"

TEST_CASE("QueueStateBuffer stalls if timestamps are too old", "[QueueStateBuffer]") {
  tr::QueueStateBuffer buffer;

  tr::Timestamp base = std::chrono::steady_clock::now();

  // Producer fills the buffer with old timestamps
  for (int i = 0; i < BufferSize; ++i) {
    tr::SimState* slot = buffer.getWriteSlot();
    REQUIRE(slot != nullptr);
    slot->timeStamp = base + std::chrono::milliseconds(i * 10);
    slot->value = i;
    buffer.commitWrite();
  }

  // Simulate the consumer waking up *much later*
  tr::Timestamp currentTime = base + std::chrono::milliseconds(100);

  tr::SimState a, b;
  float alpha;

  bool result = buffer.getInterpolatedStates(a, b, alpha, currentTime);

  REQUIRE_FALSE(result); // Should fail to interpolate — everything is too old
}
