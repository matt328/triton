#include "leaf.hpp"

#include <iostream>
#include <deque>
#include <cstdlib>
#include <stacktrace>

#define ENABLE_ERROR_TRACE 1

#define CLASS_FUNCTION_NAME() (std::string("Function: ") + __func__)

namespace leaf = boost::leaf;

// The error trace is activated only if an error handling scope provides a
// handler for e_error_trace.
struct e_error_trace {
   struct rec {
      char const* file;
      int line;
      std::string loc;
      friend std::ostream& operator<<(std::ostream& os, rec const& x) {
         return os << x.loc << ": " << x.file << '(' << x.line << ')' << std::endl;
      }
   };

   std::deque<rec> value;

   friend std::ostream& operator<<(std::ostream& os, e_error_trace const& tr) {
      for (auto& i : tr.value)
         os << i;
      return os;
   }
};

// The ERROR_TRACE macro is designed for use in functions that detect or forward
// errors up the call stack. If an error occurs, and if an error handling scope
// provides a handler for e_error_trace, the supplied lambda is executed as the
// error bubbles up.
#define ERROR_TRACE                                                                                \
   auto _trace = leaf::on_error([](e_error_trace& tr) {                                            \
      tr.value.emplace_front(                                                                      \
          e_error_trace::rec{__FILE__, __LINE__, std::to_string(std::stacktrace::current())});     \
   })

// Each function in the sequence below calls the previous function, and each
// function has failure_percent chance of failing. If a failure occurs, the
// ERROR_TRACE macro will cause the path the error takes to be captured in an
// e_error_trace.
int const failure_percent = 25;

leaf::result<void> f1() {
   ERROR_TRACE;
   if ((std::rand() % 100) > failure_percent)
      return {};
   else
      return leaf::new_error();
}

leaf::result<void> f2() {
   ERROR_TRACE;
   if ((std::rand() % 100) > failure_percent)
      return f1();
   else
      return leaf::new_error();
}

leaf::result<void> f3() {
   ERROR_TRACE;
   if ((std::rand() % 100) > failure_percent)
      return f2();
   else
      return leaf::new_error();
}

leaf::result<void> f4() {
   ERROR_TRACE;
   if ((std::rand() % 100) > failure_percent)
      return f3();
   else
      return leaf::new_error();
}

leaf::result<void> f5() {
   ERROR_TRACE;
   if ((std::rand() % 100) > failure_percent)
      return f4();
   else
      return leaf::new_error();
}

void baz() {
   std::cout << std::to_string(std::stacktrace::current()) << std::endl;
}

void bar() {
   baz();
}

void foo() {
   bar();
}

int main() {
   foo();
   //    for (int i = 0; i != 10; ++i)
   //       leaf::try_handle_all(
   //           [&]() -> leaf::result<void> {
   //              std::cout << "Run # " << i << ": ";
   //              BOOST_LEAF_CHECK(f5());
   //              std::cout << "Success!" << std::endl;
   //              return {};
   //           },
   // #if ENABLE_ERROR_TRACE // This single #if enables or disables the capturing of the error
   // trace.
   //           [](e_error_trace const& tr) { std::cerr << "Error! Trace:" << std::endl
   //                                                   << tr; },
   // #endif
   //           [] { std::cerr << "Error!" << std::endl; });
   return 0;
}
