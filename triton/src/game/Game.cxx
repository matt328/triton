#include "Game.hpp"

#include "Renderable.hpp"
#include "Transform.hpp"
#include "core/Utils.hpp"
#include "RenderSystem.hpp"
#include "TransformSystem.hpp"
#include "game/InputSystem.hpp"
#include "graphics/RenderDevice.hpp"

Game::Game(RenderDevice& renderDevice) {
   registry = std::make_unique<entt::registry>();

   renderSystem = std::make_shared<RenderSystem>();
   transformSystem = std::make_unique<TransformSystem>();
   inputSystem = std::make_unique<InputSystem>();

   renderDevice.registerRenderSystem(renderSystem);

   // Create viking room entity
   const auto textureFilename = (Core::Paths::TEXTURES / "viking_room.png").string();
   const auto filename = (Core::Paths::MODELS / "viking_room.gltf").string();

   const auto meshId = renderDevice.createMesh(filename);
   const auto textureId = renderDevice.createTexture(textureFilename);

   const auto room = registry->create();
   registry->emplace<Renderable>(room, meshId, textureId);
   registry->emplace<Transform>(room);

   // Create Plane area entity
   const auto planeMeshId = renderDevice.createMesh((Core::Paths::MODELS / "area.gltf").string());
   const auto planeTextureId =
       renderDevice.createTexture((Core::Paths::TEXTURES / "grass.png").string());

   const auto floor = registry->create();
   registry->emplace<Renderable>(floor, planeMeshId, planeTextureId);
   registry->emplace<Transform>(floor);

   inputSystem->getActionDelegate().connect<&TransformSystem::handleAction>(transformSystem);
}

void Game::keyPressed(const int key, int scancode, const int action, int mods) const {
   inputSystem->keyCallback(key, scancode, action, mods);
}
void Game::update() const {
   transformSystem->update(*registry);
   renderSystem->update(*registry);
}

void Game::blendState(double alpha) {
   // state = currentState * alpha + previousState * (1.0 - alpha);

   // Log::core->debug("blendState(), alpha: {}", alpha);
}
