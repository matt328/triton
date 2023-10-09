#include <sol/protected_function_result.hpp>
#include <sol/state_handling.hpp>
#define SOL_ALL_SAFETIES_ON 1

#include <sol/sol.hpp>

#include <iostream>
#include <cassert>

int main() {
   std::cout << "opening a state" << std::endl;

   sol::state lua;

   lua.open_libraries(sol::lib::base, sol::lib::package);

   lua.script_file("fooscript.lua");

   std::function<int(int, double, int, std::string)> updateFn = lua["update"];

   int is_one = updateFn(1, 34.5, 3, "bark");
   assert(is_one == 1);

   return 0;
}
