#pragma once

#include "bk/Handle.hpp"

namespace tr {

template <typename T>
struct HandleGenerator {

  HandleGenerator() : gen{randomDevice()}, distribution(1, 10000) {
  }

  [[nodiscard]] auto requestHandle() -> Handle<T> {
    return Handle<T>{.id = distribution(gen)};
  }

  [[nodiscard]] auto requestLogicalHandle() -> LogicalHandle<T> {
    return LogicalHandle<T>{.id = distribution(gen)};
  }

private:
  std::random_device randomDevice;
  std::mt19937 gen;
  std::uniform_int_distribution<std::size_t> distribution;
};

}
