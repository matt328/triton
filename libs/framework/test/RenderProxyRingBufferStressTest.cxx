#include "fx/RenderProxyRingBuffer.hpp"

TEST_CASE("RenderProxyRingBuffer concurrent stress test", "[multithreaded]") {
  constexpr size_t BufferSize = 32;
  tr::RenderProxyRingBuffer<int, BufferSize> buffer;

  std::atomic<bool> done = false;
  std::atomic<uint64_t> frameCounter = 0;

  // Producer thread: Push incrementing frame indices
  std::thread producer([&]() {
    for (uint64_t i = 0; i < 10000; ++i) {
      buffer.pushState(static_cast<int>(i * 10), i); // state = 10 * frameIndex
      frameCounter.store(i, std::memory_order_relaxed);
    }
    std::this_thread::sleep_for(std::chrono::microseconds(200000));
    done = true;
  });

  // std::this_thread::sleep_for(std::chrono::microseconds(100));

  // Consumer thread: Randomly read nearby frame indices
  std::thread consumer([&]() {
    std::mt19937 rng(42);
    std::uniform_int_distribution<int> offset(-2, 2);

    while (!done) {
      uint64_t latestFrame = frameCounter.load(std::memory_order_relaxed);
      if (latestFrame < 2)
        continue;

      uint64_t target = latestFrame + offset(rng);

      auto result = buffer.getStates(target);
      if (result) {
        auto [a, b] = *result;

        CAPTURE(target);
        CAPTURE(a);
        CAPTURE(b);

        CHECK(a <= b);                                 // sanity check
        CHECK((a <= target * 10 && target * 10 <= b)); // state is 10 * frameIndex
      }

      std::this_thread::sleep_for(std::chrono::microseconds(20));
    }
  });

  producer.join();
  consumer.join();
}
