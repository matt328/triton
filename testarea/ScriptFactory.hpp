#pragma once

#include <sol/state.hpp>

class FooScript;

class ScriptFactory {
 public:
   ScriptFactory();
   ~ScriptFactory() = default;

   ScriptFactory(const ScriptFactory&) = delete;
   ScriptFactory(ScriptFactory&&) = delete;
   ScriptFactory& operator=(const ScriptFactory&) = delete;
   ScriptFactory& operator=(ScriptFactory&&) = delete;

   std::unique_ptr<FooScript> createFooScript(std::string filename);

 private:
   sol::state lua;
};
