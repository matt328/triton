#include "Game.hpp"

#include "Renderable.hpp"
#include "core/Utils.hpp"
#include "RenderSystem.hpp"
#include "graphics/RenderDevice.hpp"

Game::Game(RenderDevice& renderDevice) {
   registry = std::make_unique<entt::registry>();

   renderSystem = std::make_unique<RenderSystem>(renderDevice);

   const auto textureFilename = (Core::Paths::TEXTURES / "viking_room.png").string();
   const auto filename = (Core::Paths::MODELS / "viking_room.gltf").string();

   const auto meshId = renderDevice.createMesh(filename);
   const auto textureId = renderDevice.createTexture(textureFilename);

   const auto room = registry->create();
   registry->emplace<Renderable>(room, meshId, textureId);
}

void Game::update(double t, const float dt) const {
   renderSystem->update(*registry, dt);
}

void Game::blendState(double alpha) {
   // state = currentState * alpha + previousState * (1.0 - alpha);

   // Log::core->debug("blendState(), alpha: {}", alpha);
}
