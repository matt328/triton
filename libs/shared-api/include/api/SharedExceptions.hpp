#pragma once

namespace tr {
class GeometryDataNotFoundException : public std::logic_error {
public:
  explicit GeometryDataNotFoundException(const std::string& message) : std::logic_error(message) {
  }
};

class IOException final : public tr::BaseException {
public:
  using BaseException::BaseException;
};
}
