#include "Game.hpp"

#include "core/Paths.hpp"
#include "game/actions/ActionSet.hpp"
#include "game/actions/ActionSystem.hpp"
#include "game/actions/ActionType.hpp"
#include "game/actions/Sources.hpp"
#include "game/ecs/component/Resources.hpp"

#include "graphics/Renderer.hpp"

#include "game/ecs/component/Renderable.hpp"
#include "game/ecs/component/Camera.hpp"
#include "game/ecs/component/Transform.hpp"
#include "game/ecs/system/RenderSystem.hpp"
#include "game/ecs/system/CameraSystem.hpp"

namespace Triton::Game {

   using namespace Actions;

   // HACK: This entire class.  slopping stuff in here to manually test out the renderer before
   // adding proper ECS.
   Game::Game(GLFWwindow* window) {
      int width{}, height{};
      glfwGetWindowSize(window, &width, &height);

      renderer = std::make_unique<Graphics::Renderer>(window);

      registry = std::make_unique<entt::registry>();

      actionSystem = std::make_unique<ActionSystem>(*window);

      registry->ctx().emplace<ActionState&>(actionSystem->getActionState());

      auto& map = actionSystem->createActionSet(ActionSets::Main);

      // map.addBinding(ActionType::MoveForward, {Source{Key::Up}, Source{Key::W}});

      map.bindSource(Source{Key::Up}, ActionType::MoveForward);
      map.bindSource(Source{Key::W}, ActionType::MoveForward);

      map.bindSource(Source{Key::Down}, ActionType::MoveBackward);
      map.bindSource(Source{Key::S}, ActionType::MoveBackward);

      map.bindSource(Source{Key::Right}, ActionType::StrafeRight);
      map.bindSource(Source{Key::D}, ActionType::StrafeRight);

      map.bindSource(Source{Key::Left}, ActionType::StrafeLeft);
      map.bindSource(Source{Key::A}, ActionType::StrafeLeft);

      map.bindSource(Source{MouseInput::MOVE_X}, ActionType::LookHorizontal);
      map.bindSource(Source{MouseInput::MOVE_Y}, ActionType::LookVertical);

      // Create viking room entity
      const auto textureFilename = (Core::Paths::TEXTURES / "grass.png").string();
      const auto filename = (Core::Paths::MODELS / "area.gltf").string();

      const auto meshId = renderer->createMesh(filename);
      const auto textureId = renderer->createTexture(textureFilename);

      const auto room = registry->create();
      registry->emplace<Ecs::Renderable>(room, meshId, textureId);
      registry->emplace<Ecs::Transform>(room);

      const auto camera = registry->create();
      registry->emplace<Ecs::Camera>(camera, width, height, 60.f, 0.1f, 1000.f);

      registry->ctx().emplace<Ecs::WindowDimensions>(width, height);
      registry->ctx().emplace<Ecs::CurrentCamera>(camera);
   }

   Game::~Game() {
      Log::info << "destroying game" << std::endl;
   };

   void Game::beginFrame() {
      TracyMessageL("beginFrame");
      actionSystem->update();
   }

   void Game::fixedUpdate([[maybe_unused]] const Core::Timer& timer) {
      TracyMessageL("fixedUpdate");
      ZoneNamedN(upd, "FixedUpdate", true);
      Ecs::CameraSystem::fixedUpdate(*registry);
   }

   void Game::update() {
      TracyMessageL("render");
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