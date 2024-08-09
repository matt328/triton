#include <utility>

class BaseException : public std::exception {
 public:
   explicit BaseException(std::string message) : message(std::move(message)) {
   }

   explicit BaseException(const std::string& message, const BaseException& other)
       : message(message + other.what()) {
   }

   [[nodiscard]] const char* what() const noexcept override {
      return message.c_str();
   }

   BaseException& operator<<(const std::string& additionalInfo) {
      message = additionalInfo + message;
      return *this;
   }

 private:
   std::string message;
};

class MyException final : public BaseException {
 public:
   using BaseException::BaseException;
};

class MyOtherException final : public BaseException {
 public:
   using BaseException::BaseException;
};

void foo() {
   throw MyException("foo(): something broke");
}

void bar() {
   try {
      foo();
   } catch (MyException& ex) {
      ex << "bar(): ";
      throw;
   }
}

void baz() {
   try {
      bar();
   } catch (BaseException& ex) { throw MyOtherException("baz(): ", ex); }
}

int main() {

   try {
      baz();
   } catch (const MyOtherException& ex) { std::cout << ex.what(); }

   return 0;
}
