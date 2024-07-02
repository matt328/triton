#include <futures/futures.hpp>

int main() {

   auto f1 = futures::async([]() -> int { return 42; });
   auto f1_cont = then(f1, [](int x) { return x * 2; });

   assert(f1_cont.get() == 84);

   return 0;
}
