#pragma once

#include <entt/fwd.hpp>

namespace tr {
   using EntityType = entt::entity;
   using TerrainCreatedFn = std::function<void(EntityType)>;
}
