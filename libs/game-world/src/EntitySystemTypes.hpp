#pragma once

// Move this to be internal to entt-game-world and change EntityType back to entt::entity
// game-world-api should convert between this and GameWorldObjectId

namespace tr {
using EntityType = entt::entity;
using TerrainCreatedFn = std::function<void(EntityType)>;
}
