#include "Game.hpp"

#include "core/Paths.hpp"
#include "game/ecs/component/Resources.hpp"
#include "graphics/RenderObject.hpp"
#include "graphics/Renderer.hpp"

#include "game/ecs/component/Renderable.hpp"
#include "game/ecs/component/Camera.hpp"
#include "game/ecs/component/Transform.hpp"
#include "game/ecs/system/RenderSystem.hpp"
#include "game/ecs/system/CameraSystem.hpp"

namespace Triton::Game {
   // HACK: This entire class.  slopping stuff in here to manually test out the renderer before
   // adding proper ECS.
   Game::Game(GLFWwindow* window) {
      renderer = std::make_unique<Graphics::Renderer>(window);

      registry = std::make_unique<entt::registry>();

      // Create viking room entity
      const auto textureFilename = (Core::Paths::TEXTURES / "viking_room.png").string();
      const auto filename = (Core::Paths::MODELS / "viking_room.gltf").string();

      const auto meshId = renderer->createMesh(filename);
      const auto textureId = renderer->createTexture(textureFilename);

      const auto room = registry->create();
      registry->emplace<Ecs::Renderable>(room, meshId, textureId);
      registry->emplace<Ecs::Transform>(room);

      int width{}, height{};
      glfwGetWindowSize(window, &width, &height);
      const auto camera = registry->create();
      registry->emplace<Ecs::Camera>(camera, width, height, 60.f, 0.1f, 1000.f);

      registry->ctx().emplace<Ecs::WindowDimensions>(width, height);
      registry->ctx().emplace<Ecs::CurrentCamera>(camera);

      renderer->registerPerFrameDataProvider([] {
         const auto perFrameData = Graphics::PerFrameData{
             .view = glm::identity<glm::mat4>(),
             .proj = glm::identity<glm::mat4>(),
             .viewProj = glm::identity<glm::mat4>(),
         };
         return perFrameData;
      });
   }

   Game::~Game() {
      Log::info << "destroying game" << std::endl;
   };

   void Game::update([[maybe_unused]] const Core::Timer& timer) {
      ZoneNamedN(upd, "FixedUpdate", true);
      Ecs::CameraSystem::update(*registry);
   }

   void Game::render() {
      Ecs::RenderSystem::update(*registry, *renderer);
      renderer->render();
   }

   void Game::resize(const int width, const int height) {
      renderer->windowResized(width, height);
      registry->ctx().emplace<Ecs::WindowDimensions>(width, height);
   }

   void Game::waitIdle() {
      renderer->waitIdle();
   }
}