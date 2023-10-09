#include "ScriptFactory.hpp"

ScriptFactory::ScriptFactory() {
   lua.open_libraries(sol::lib::base);
}

std::unique_ptr<FooScript> ScriptFactory::createFooScript(std::string filename) {
   lua.script_file(filename);
}
