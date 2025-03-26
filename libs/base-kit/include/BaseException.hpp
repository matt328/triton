#pragma once

namespace tr {

class BaseException : public std::exception {
public:
  explicit BaseException(std::string message) : message(std::move(message)) {
  }

  explicit BaseException(const std::string& message, const std::exception& other)
      : message(message + other.what()) {
  }

  [[nodiscard]] auto what() const noexcept -> const char* override {
    return message.c_str();
  }

  auto operator<<(const std::string& additionalInfo) -> BaseException& {
    message = additionalInfo + message;
    return *this;
  }

private:
  std::string message{};
};

};
