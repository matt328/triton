#pragma once

namespace tr {

template <typename T>
struct TaggedSlot {
  uint64_t tag; // Frame Number of timestamp
  T data;
};

template <typename T, size_t Capacity>
class LockFreeRingBuffer {
  static_assert(Capacity > 0 && (Capacity & (Capacity - 1)) == 0, "Capacity must be a power of 2");

public:
  LockFreeRingBuffer() {
    for (size_t i = 0; i < Capacity; ++i) {
      buffer[i] = std::make_unique<T>();
    }
  }

  // Push fails if buffer is full
  auto push(const T& value) -> bool {
    size_t nextHead = (head + 1) & (Capacity - 1);
    if (nextHead == tail.load(std::memory_order_acquire)) {
      return false; // Full
    }

    *buffer[head] = value;
    head = nextHead;
    return true;
  }

  // Pop fails if buffer is empty
  auto pop(T& out) -> bool {
    if (tail.load(std::memory_order_acquire) == head) {
      return false; // Empty
    }

    out = *buffer[tail];
    tail.store((tail + 1) & (Capacity - 1), std::memory_order_release);
    return true;
  }

  auto peekWritable() -> T* {
    size_t nextHead = (head + 1) & (Capacity - 1);
    if (nextHead == tail.load(std::memory_order_acquire)) {
      return nullptr; // Full
    }
    return buffer[head].get();
  }

  void commitWrite() {
    head = (head + 1) & (Capacity - 1);
  }

  auto peekReadable() const -> T* {
    if (tail.load(std::memory_order_acquire) == head) {
      return nullptr; // Empty
    }
    return buffer[tail].get();
  }

  void commitRead() {
    tail.store((tail + 1) & (Capacity - 1), std::memory_order_release);
  }

private:
  std::array<std::unique_ptr<T>, Capacity> buffer;
  std::atomic<size_t> head = 0;
  size_t tail = 0;
};

}
