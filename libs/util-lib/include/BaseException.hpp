#pragma once

namespace tr {

   class BaseException : public std::exception {
    public:
      explicit BaseException(std::string message) : message(std::move(message)) {
      }

      explicit BaseException(const std::string& message, const std::exception& other)
          : message(std::move(message + other.what())) {
      }

      [[nodiscard]] const char* what() const noexcept override {
         return message.c_str();
      }

      BaseException& operator<<(const std::string& additionalInfo) {
         message = additionalInfo + message;
         return *this;
      }

    private:
      std::string message{};
   };

};
