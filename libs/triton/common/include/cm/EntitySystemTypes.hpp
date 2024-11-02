#pragma once

#include <entt/fwd.hpp>

namespace tr::cm {
   using EntityType = entt::entity;
   using TerrainCreatedFn = std::function<void(cm::EntityType)>;
} // namespace tr::cm
