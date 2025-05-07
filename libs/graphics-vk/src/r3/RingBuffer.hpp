#pragma once

#include "api/gfx/GpuMaterialData.hpp"
namespace tr {

struct SimState {
  double timeStamp;
  std::vector<GpuObjectData> objectMetadata;
  std::vector<glm::vec3> positions;
  std::vector<glm::quat> rotations;
  std::vector<glm::vec3> scales;

  // Pre-allocate memory for the vectors when creating a SimState
  explicit SimState(size_t numObjects)
      : objectMetadata(numObjects),
        positions(numObjects),
        rotations(numObjects),
        scales(numObjects) {
  }
};

class RingBuffer {
public:
  RingBuffer(const RingBuffer&) = delete;
  auto operator=(const RingBuffer&) -> RingBuffer& = delete;
  RingBuffer(RingBuffer&&) = delete;
  auto operator=(RingBuffer&&) -> RingBuffer& = delete;

  ~RingBuffer() {
    buffer.clear();
  }

  // Constructor to initialize the buffer with a fixed size.
  explicit RingBuffer(size_t capacity, size_t numObjectsPerState)
      : size(capacity), readIndex(0), writeIndex(0) {
    assert(capacity > 1); // Capacity must be at least 2

    // Pre-allocate memory for each SimState's vectors upfront
    buffer.reserve(capacity);
    for (size_t i = 0; i < capacity; ++i) {
      buffer.emplace_back(numObjectsPerState); // call SimState(size_t)
    }
  }

  // Insert data into the ring buffer
  auto write(const SimState& state) -> bool {
    size_t currentWrite = writeIndex.load(std::memory_order_relaxed);
    size_t nextWrite = (currentWrite + 1) % size;

    if (nextWrite != readIndex.load(std::memory_order_acquire)) {
      // Insert the SimState into the buffer (no dynamic allocation needed)
      buffer[currentWrite] = state; // Copy the state into the buffer at currentWrite position
      writeIndex.store(nextWrite, std::memory_order_release);
      return true;
    }

    // Buffer is full
    return false;
  }

  // Read data from the ring buffer
  auto read(SimState& result) -> bool {
    size_t currentRead = readIndex.load(std::memory_order_relaxed);
    if (currentRead == writeIndex.load(std::memory_order_acquire)) {
      // Buffer is empty
      return false;
    }

    size_t nextRead = (currentRead + 1) % size;
    result = buffer[currentRead]; // Copy the state from the buffer
    readIndex.store(nextRead, std::memory_order_release);
    return true;
  }

  // Return the item at the front of the buffer without removing it
  auto frontPtr() -> SimState* {
    size_t currentRead = readIndex.load(std::memory_order_relaxed);
    if (currentRead == writeIndex.load(std::memory_order_acquire)) {
      // Buffer is empty
      return nullptr;
    }
    return &buffer[currentRead];
  }

  // Pop the front item (similar to a read, but without returning it)
  void popFront() {
    size_t currentRead = readIndex.load(std::memory_order_relaxed);
    assert(currentRead != writeIndex.load(std::memory_order_acquire));

    size_t nextRead = (currentRead + 1) % size;
    readIndex.store(nextRead, std::memory_order_release);
  }

  auto isEmpty() const -> bool {
    return readIndex.load(std::memory_order_acquire) == writeIndex.load(std::memory_order_acquire);
  }

  auto isFull() const -> bool {
    size_t nextWrite = (writeIndex.load(std::memory_order_acquire) + 1) % size;
    return nextWrite == readIndex.load(std::memory_order_acquire);
  }

  auto sizeGuess() const -> size_t {
    ssize_t diff =
        writeIndex.load(std::memory_order_acquire) - readIndex.load(std::memory_order_acquire);
    if (diff < 0) {
      diff += size;
    }
    return static_cast<size_t>(diff);
  }

  auto capacity() const -> size_t {
    return size - 1; // One slot is reserved to detect full condition
  }

  auto getInterpolatedStates(SimState& stateA,
                             SimState& stateB,
                             float& alpha,
                             double currentTimeSec) -> bool {
    size_t size = buffer.size();
    size_t latest = (readIndex.load(std::memory_order_acquire) + size - 1) % size;
    size_t previous = (latest + size - 1) % size;

    if (latest == writeIndex || previous == writeIndex) {
      return false; // Not enough data
    }

    stateA = buffer[previous];
    stateB = buffer[latest];

    double tA = stateA.timeStamp;
    double tB = stateB.timeStamp;

    if (tB <= tA || currentTimeSec <= tA) {
      alpha = 0.0f;
    } else if (currentTimeSec >= tB) {
      alpha = 1.0f;
    } else {
      alpha = static_cast<float>((currentTimeSec - tA) / (tB - tA));
    }

    return true;
  }

private:
  size_t size;
  std::vector<SimState> buffer; // Ring buffer to hold SimState objects
  std::atomic<size_t> readIndex;
  std::atomic<size_t> writeIndex;
};
}
