// #include "fx/HorribleStateBuffer.hpp"

// using namespace std::chrono;

// TEST_CASE("HorribleStateBuffer basic interpolation", "[HorribleStateBuffer]") {
//   tr::HorribleStateBuffer buffer;

//   const auto base = steady_clock::now();

//   auto state10 = tr::SimState{1};
//   state10.tag = 10;
//   auto state20 = tr::SimState{1};
//   state20.tag = 20;
//   auto state30 = tr::SimState{1};
//   state30.tag = 30;
//   auto state40 = tr::SimState{1};
//   state40.tag = 40;

//   // Push states at increasing timestamps
//   buffer.pushState(state10, base + milliseconds(100));
//   buffer.pushState(state20, base + milliseconds(200));
//   buffer.pushState(state30, base + milliseconds(300));
//   buffer.pushState(state40, base + milliseconds(400));

//   SECTION("Interpolates between frames correctly") {
//     auto result = buffer.getStates(base + milliseconds(150));
//     REQUIRE(result.has_value());
//     CHECK(result->first.tag == 10);
//     CHECK(result->second.tag == 20);
//   }

//   SECTION("Fails if frame is not bracketed") {
//     auto tooOld = buffer.getStates(base + milliseconds(0));
//     auto tooNew = buffer.getStates(base + milliseconds(500));
//     REQUIRE_FALSE(tooOld.has_value());
//     REQUIRE_FALSE(tooNew.has_value());
//   }
// }

// TEST_CASE("HorribleStateBuffer handles wraparound", "[HorribleStateBuffer]") {
//   tr::HorribleStateBuffer buffer;

//   auto state10 = tr::SimState{1};
//   state10.tag = 10;
//   auto state20 = tr::SimState{1};
//   state20.tag = 20;
//   auto state30 = tr::SimState{1};
//   state30.tag = 30;
//   auto state40 = tr::SimState{1};
//   state40.tag = 40;
//   auto state50 = tr::SimState{1};
//   state50.tag = 50;
//   auto state60 = tr::SimState{1};
//   state60.tag = 60;
//   auto state70 = tr::SimState{1};
//   state70.tag = 70;
//   auto state80 = tr::SimState{1};
//   state80.tag = 80;

//   const auto base = steady_clock::now();

//   buffer.pushState(state10, base + milliseconds(100));
//   buffer.pushState(state20, base + milliseconds(200));
//   buffer.pushState(state30, base + milliseconds(300));
//   buffer.pushState(state40, base + milliseconds(400));
//   buffer.pushState(state50, base + milliseconds(500));
//   buffer.pushState(state60, base + milliseconds(600)); // Buffer is now full

//   buffer.pushState(state70, base + milliseconds(700));
//   buffer.pushState(state80, base + milliseconds(800));

//   SECTION("Interpolates between latest valid frames") {
//     auto result = buffer.getStates(base + milliseconds(750));
//     REQUIRE(result.has_value());
//     CHECK(result->first.tag == 70);
//     CHECK(result->second.tag == 80);
//   }

//   SECTION("Fails if target frame was overwritten") {
//     auto result = buffer.getStates(base + milliseconds(150));
//     REQUIRE_FALSE(result.has_value());
//   }
// }

// TEST_CASE("HorribleStateBuffer handles empty and partial state", "[HorribleStateBuffer]") {
//   tr::HorribleStateBuffer buffer;
//   const auto base = std::chrono::steady_clock::now();

//   auto state10 = tr::SimState{1};
//   state10.tag = 10;
//   auto state20 = tr::SimState{1};
//   state20.tag = 20;

//   SECTION("Fails when buffer is empty") {
//     auto result = buffer.getStates(base + std::chrono::milliseconds(100));
//     REQUIRE_FALSE(result.has_value());
//   }

//   SECTION("Fails when only one entry exists") {
//     buffer.pushState(state10, base + std::chrono::milliseconds(100));
//     auto result = buffer.getStates(base + std::chrono::milliseconds(100));
//     REQUIRE_FALSE(result.has_value());
//   }

//   SECTION("Works with two entries bracketing the timestamp") {
//     buffer.pushState(state10, base + std::chrono::milliseconds(100));
//     buffer.pushState(state20, base + std::chrono::milliseconds(200));

//     auto result = buffer.getStates(base + std::chrono::milliseconds(150));
//     REQUIRE(result.has_value());
//     CHECK(result->first.tag == 10);
//     CHECK(result->second.tag == 20);
//   }

//   SECTION("Fails when timestamp is before the first entry") {
//     buffer.pushState(state10, base + std::chrono::milliseconds(100));
//     buffer.pushState(state20, base + std::chrono::milliseconds(200));

//     auto result = buffer.getStates(base + std::chrono::milliseconds(50));
//     REQUIRE_FALSE(result.has_value());
//   }

//   SECTION("Fails when timestamp is after the last entry") {
//     buffer.pushState(state10, base + std::chrono::milliseconds(100));
//     buffer.pushState(state20, base + std::chrono::milliseconds(200));

//     auto result = buffer.getStates(base + std::chrono::milliseconds(250));
//     REQUIRE_FALSE(result.has_value());
//   }
// }
