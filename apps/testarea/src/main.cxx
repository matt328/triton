#include "QueueStateBuffer.hpp"

auto main() -> int {
  QueueStateBuffer queue;

  Timestamp startTime = std::chrono::steady_clock::now();
  double currentSimValue = 0.0;
  int timeStepMs = 16; // 60 Hz

  std::vector<SimState> producedStates;

  while (true) {
    std::cout << "\n=== MENU ===\n"
              << "1. Produce SimState\n"
              << "2. Try Interpolate\n"
              << "3. Dump Queue State\n"
              << "4. Advance time N ms\n"
              << "5. Quit\n> ";

    int choice;
    std::cin >> choice;

    if (choice == 1) {
      SimState* slot = queue.getWriteSlot();
      if (!slot) {
        std::cout << "Queue full. Cannot write.\n";
        continue;
      }

      Timestamp simTime = startTime + std::chrono::milliseconds(timeStepMs * producedStates.size());

      slot->value = currentSimValue;
      slot->timeStamp = simTime;
      queue.commitWrite();

      std::cout
          << "Produced SimState{ value=" << currentSimValue << ", time="
          << std::chrono::duration_cast<std::chrono::milliseconds>(simTime - startTime).count()
          << "ms }\n";

      currentSimValue += 1.0;
      producedStates.push_back(*slot);
    }

    else if (choice == 2) {
      std::cout << "Enter current time (ms since start): ";
      int t;
      std::cin >> t;

      Timestamp now = startTime + std::chrono::milliseconds(t);
      SimState a, b;
      float alpha;

      if (queue.getInterpolatedStates(a, b, alpha, now)) {
        auto ta =
            std::chrono::duration_cast<std::chrono::milliseconds>(a.timeStamp - startTime).count();
        auto tb =
            std::chrono::duration_cast<std::chrono::milliseconds>(b.timeStamp - startTime).count();
        std::cout << "Interpolated between t=" << ta << "ms (value=" << a.value << ")"
                  << " and t=" << tb << "ms (value=" << b.value << ")"
                  << " with alpha=" << std::fixed << std::setprecision(2) << alpha << "\n";
      } else {
        std::cout << "Interpolation failed. Not enough data or time out of range.\n";
      }
    }

    else if (choice == 3) {
      std::cout << "--- Queue Dump ---\n";
      int r = queue.readIndex;
      int w = queue.writeIndex.load(std::memory_order_acquire);
      std::cout << "readIndex=" << r << ", writeIndex=" << w << "\n";

      int i = r;
      while (i != w) {
        const SimState& s = queue.buffer[i];
        int timeMs =
            std::chrono::duration_cast<std::chrono::milliseconds>(s.timeStamp - startTime).count();
        std::cout << "[" << i << "] t=" << timeMs << "ms, value=" << s.value << "\n";
        i = (i + 1) % QueueStateBuffer::BufferSize;
      }

      if (r == w)
        std::cout << "Buffer empty.\n";
    }

    else if (choice == 4) {
      std::cout << "Enter milliseconds to advance: ";
      int advance;
      std::cin >> advance;
      startTime -= std::chrono::milliseconds(advance);
      std::cout << "Time shifted forward by " << advance << "ms.\n";
    }

    else if (choice == 5) {
      break;
    }
  }

  return 0;
}
