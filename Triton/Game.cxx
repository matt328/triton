#include "pch.hpp"

#include "Game.hpp"

#include "Renderable.hpp"
#include "Utils.hpp"
#include "RenderSystem.hpp"
#include "Transform.hpp"
#include "RenderDevice.hpp"

Game::Game(RenderDevice& renderDevice) {
   registry = std::make_unique<entt::registry>();

   renderSystem = std::make_unique<RenderSystem>(renderDevice);

   for (auto i = 0u; i < 10u; ++i) {
      const auto entity = registry->create();
      registry->emplace<Transform>(entity, glm::mat4(1.f));
   }

   const auto filename = (Paths::MODELS / "viking_room.gltf").string();

   const auto id = renderDevice.createMesh(filename.data());

   const auto room = registry->create();
   registry->emplace<Renderable>(room, filename.data());

   // level loading or game logic needs to tell the renderdevice to load a model, then
   // return an opaque token for the asset, then put that token in the Renderable
   // for the entity.
   // Then the RenderSystem will enqueue this thing to be rendered

   // TODO: need a way for the game to call into the renderDevice, probably renderSystem
}

void Game::update(double t, const float dt) const {
   // previousState = currentState;
   // Update state
   // Log::core->debug("update() t: {}, dt: {}", t, dt);

   renderSystem->update(*registry, dt);

   for (const auto view = registry->view<const Transform>();
        auto [entity, transform] : view.each()) {
      // Log::game->info("Updating entity {}", glm::to_string(transform.getPosition()));
   }
}

void Game::blendState(double alpha) {
   // state = currentState * alpha + previousState * (1.0 - alpha);

   // Log::core->debug("blendState(), alpha: {}", alpha);
}
