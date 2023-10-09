#pragma once

class ScriptingSystem {
 public:
   ScriptingSystem() = default;
   ~ScriptingSystem() = default;

   ScriptingSystem(const ScriptingSystem&) = default;
   ScriptingSystem(ScriptingSystem&&) = delete;
   ScriptingSystem& operator=(const ScriptingSystem&) = default;
   ScriptingSystem& operator=(ScriptingSystem&&) = delete;

   void update(entt::registry& registry);
};