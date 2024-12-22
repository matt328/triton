#include "gp/Registry.hpp"
#include <entt/entity/fwd.hpp>

namespace tr {
   Registry::Registry() {
      registry = std::make_unique<entt::registry>();
   }

   Registry::~Registry() {
      Log.trace("Destroyed Registry");
   }

   auto Registry::getRegistry() const -> entt::registry& {
      return *registry;
   }

   auto Registry::getConstRegistry() const -> const entt::registry& {
      return *registry;
   }
}