#include "fx/HorribleStateBuffer.hpp"

TEST_CASE("HorribleStateBuffer 240Hz producer / 60Hz consumer stress test", "[multithreaded]") {
  tr::HorribleStateBuffer buffer;

  using Clock = std::chrono::steady_clock;
  using namespace std::chrono;

  const auto base = Clock::now();
  std::atomic<bool> done = false;
  std::atomic<uint64_t> frameCounter = 0;

  // Producer thread (240Hz)
  std::thread producer([&]() {
    for (uint64_t i = 0; i < 1000; ++i) {
      tr::SimState state{1};
      state.tag = static_cast<int>(i * 10);

      tr::Timestamp timestamp =
          base + duration_cast<Clock::duration>(duration<double>(i * (1.0 / 240.0)));
      buffer.pushState(state, timestamp);
      frameCounter.store(i, std::memory_order_relaxed);

      std::this_thread::sleep_for(milliseconds(4) + microseconds(160)); // ~240Hz
    }
    done = true;
  });

  // Consumer thread (60Hz)
  std::thread consumer([&]() {
    size_t successfulInterpolations = 0;

    std::mt19937 rng(42);
    std::uniform_int_distribution<int64_t> offsetDist(-3, 1);

    while (!done) {
      uint64_t latest = frameCounter.load(std::memory_order_relaxed);
      if (latest < 4) {
        std::this_thread::sleep_for(milliseconds(1));
        continue;
      }

      int64_t targetFrame = static_cast<int64_t>(latest - 4) + offsetDist(rng);
      if (targetFrame < 0)
        continue;

      // Request interpolation halfway between two frames
      auto targetTimestamp = base + duration_cast<Clock::duration>(
                                        duration<double>((targetFrame + 0.5) * (1.0 / 240.0)));

      auto result = buffer.getStates(targetTimestamp);
      CHECK(result);
      if (result) {
        auto [a, b] = *result;
        CAPTURE(targetFrame);
        CAPTURE(a.tag);
        CAPTURE(b.tag);

        CHECK(a.tag <= b.tag);
        ++successfulInterpolations;
      }

      std::this_thread::sleep_for(milliseconds(16) + microseconds(600)); // ~60Hz
    }

    CHECK(successfulInterpolations > 0); // Ensure we actually got brackets
  });

  producer.join();
  consumer.join();
}
