#include "fx/RenderProxyRingBuffer.hpp"

struct SimState {
  int value;
  auto operator==(const SimState& other) const -> bool {
    return value == other.value;
  }
};

TEST_CASE("RenderProxyRingBuffer basic interpolation", "[RenderProxyRingBuffer]") {
  tr::RenderProxyRingBuffer<SimState, 4> buffer;

  buffer.pushState({10}, 1);
  buffer.pushState({20}, 2);
  buffer.pushState({30}, 3);
  buffer.pushState({40}, 4);

  SECTION("Interpolates between frames correctly") {
    auto result = buffer.getStates(3);
    REQUIRE(result.has_value());
    CHECK(result->first.value == 30);
    CHECK(result->second.value == 40);
  }

  SECTION("Fails if frame is not bracketed") {
    auto tooOld = buffer.getStates(0);
    auto tooNew = buffer.getStates(5);
    REQUIRE_FALSE(tooOld.has_value());
    REQUIRE_FALSE(tooNew.has_value());
  }
}

TEST_CASE("RenderProxyRingBuffer handles wraparound", "[RenderProxyRingBuffer]") {
  tr::RenderProxyRingBuffer<SimState, 3> buffer;

  buffer.pushState({1}, 1);
  buffer.pushState({2}, 2);
  buffer.pushState({3}, 3); // buffer is now full

  buffer.pushState({4}, 4); // should overwrite frame 1
  buffer.pushState({5}, 5); // should overwrite frame 2

  SECTION("Interpolates between latest valid frames") {
    auto result = buffer.getStates(4);
    REQUIRE(result.has_value());
    CHECK(result->first.value == 4);
    CHECK(result->second.value == 5);
  }

  SECTION("Fails if target frame was overwritten") {
    auto result = buffer.getStates(1);
    REQUIRE_FALSE(result.has_value());
  }
}

TEST_CASE("RenderProxyRingBuffer handles empty and partial state", "[RenderProxyRingBuffer]") {
  tr::RenderProxyRingBuffer<SimState, 4> buffer;

  SECTION("Fails when buffer is empty") {
    auto result = buffer.getStates(1);
    REQUIRE_FALSE(result.has_value());
  }

  SECTION("Fails when only one entry exists") {
    buffer.pushState({10}, 1);
    auto result = buffer.getStates(1);
    REQUIRE_FALSE(result.has_value());
  }

  SECTION("Works with two entries") {
    buffer.pushState({10}, 1);
    buffer.pushState({20}, 2);
    auto result = buffer.getStates(1);
    REQUIRE(result.has_value());
    CHECK(result->first.value == 10);
    CHECK(result->second.value == 20);
  }
}
