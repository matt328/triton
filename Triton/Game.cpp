#include "Game.h"

#include <glm/gtx/string_cast.hpp>

#include "GltfModel.h"
#include "Log.h"
#include "Utils.h"

Game::Game() {
  registry = std::make_unique<entt::registry>();
  for (auto i = 0u; i < 10u; ++i) {
    const auto entity = registry->create();
    registry->emplace<Transform>(entity, glm::mat4(1.f));
    if (i % 2 == 0) {
    }
  }

  const auto filename = (Paths::MODELS / "viking_room.glb").string();

  const auto model = std::make_unique<GltfModel>(filename);
}

void Game::update(double t, const float dt) const {
  // previousState = currentState;
  // Update state
  // Log::core->debug("update() t: {}, dt: {}", t, dt);

  for (const auto view = registry->view<const Transform>();
       auto [entity, transform] : view.each()) {
    // Log::game->info("Updating entity {}", glm::to_string(transform.getPosition()));
  }
}

void Game::blendState(double alpha) {
  // state = currentState * alpha + previousState * (1.0 - alpha);

  // Log::core->debug("blendState(), alpha: {}", alpha);
}
