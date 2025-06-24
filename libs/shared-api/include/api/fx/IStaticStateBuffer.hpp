#pragma once

namespace tr {

using Timestamp = std::chrono::steady_clock::time_point;

template <typename T>
class IStaticStateBuffer {
public:
  virtual ~IStaticStateBuffer() = default;

  virtual auto getStates(Timestamp t) -> std::optional<T> = 0;
  virtual void pushState(const T& newState, Timestamp t) = 0;
};

}
