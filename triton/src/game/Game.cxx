#include "Game.hpp"

#include "game/actions/ActionType.hpp"
#include "util/Paths.hpp"
#include "game/actions/ActionSystem.hpp"
#include "game/actions/Inputs.hpp"
#include "game/actions/Action.hpp"
#include "game/actions/Sources.hpp"

#include "game/ecs/component/Resources.hpp"

#include "graphics/Renderer.hpp"

#include "game/ecs/component/Renderable.hpp"
#include "game/ecs/component/Camera.hpp"
#include "game/ecs/component/Transform.hpp"
#include "game/ecs/system/RenderSystem.hpp"
#include "game/ecs/system/CameraSystem.hpp"

namespace Triton::Game {

   using namespace Triton::Actions;

   const auto ZNear = 0.1f;
   const auto ZFar = 1000.f;
   const auto Fov = 60.f;
   const auto CamStart = glm::vec3{1.f, 1.f, 3.f};

   // HACK: This entire class.  slopping stuff in here to manually test out the renderer before
   // adding proper ECS.
   Game::Game(GLFWwindow* window)
       : registry{std::make_unique<entt::registry>()}, room{registry->create()} {
      int width{}, height{};
      glfwGetWindowSize(window, &width, &height);

      renderer = std::make_unique<Graphics::Renderer>(window);

      actionSystem = std::make_unique<ActionSystem>();

      auto& reg = *registry;

      actionSystem->getDelegate().connect<&Ecs::CameraSystem::handleAction>(reg);

      // Forward
      actionSystem->mapSource(Source{Key::Up, SourceType::Boolean},
                              StateType::State,
                              ActionType::MoveForward);
      actionSystem->mapSource(Source{Key::W, SourceType::Boolean},
                              StateType::State,
                              ActionType::MoveForward);

      // Backward
      actionSystem->mapSource(Source{Key::Down, SourceType::Boolean},
                              StateType::State,
                              ActionType::MoveBackward);
      actionSystem->mapSource(Source{Key::S, SourceType::Boolean},
                              StateType::State,
                              ActionType::MoveBackward);
      // Left
      actionSystem->mapSource(Source{Key::Left, SourceType::Boolean},
                              StateType::State,
                              ActionType::StrafeLeft);
      actionSystem->mapSource(Source{Key::A, SourceType::Boolean},
                              StateType::State,
                              ActionType::StrafeLeft);
      // Right
      actionSystem->mapSource(Source{Key::Right, SourceType::Boolean},
                              StateType::State,
                              ActionType::StrafeRight);
      actionSystem->mapSource(Source{Key::D, SourceType::Boolean},
                              StateType::State,
                              ActionType::StrafeRight);
      // Look
      actionSystem->mapSource(Source{MouseInput::MOVE_X, SourceType::Float},
                              StateType::Range,
                              ActionType::LookHorizontal);
      actionSystem->mapSource(Source{MouseInput::MOVE_Y, SourceType::Float},
                              StateType::Range,
                              ActionType::LookVertical);

      // Create viking room entity
      const auto textureFilename = (Util::Paths::TEXTURES / "viking_room.png").string();
      const auto filename = (Util::Paths::MODELS / "viking_room.gltf").string();

      const auto meshId = renderer->createMesh(filename);
      const auto textureId = renderer->createTexture(textureFilename);

      registry->emplace<Ecs::Renderable>(room, meshId, textureId);
      registry->emplace<Ecs::Transform>(room);

      const auto camera = registry->create();
      registry->emplace<Ecs::Camera>(camera, width, height, Fov, ZNear, ZFar, CamStart);

      registry->ctx().emplace<Ecs::WindowDimensions>(width, height);
      registry->ctx().emplace<Ecs::CurrentCamera>(camera);

      entityEditor = std::make_unique<MM::EntityEditor<entt::entity>>();

      entityEditor->registerComponent<Ecs::Transform>("Transform");
      entityEditor->registerComponent<Ecs::Renderable>("Renderable");
      entityEditor->registerComponent<Ecs::Camera>("Camera");

      renderer->getResizeDelegate().connect<&Game::resize>(this);
   }

   Game::~Game() {
      Log::info << "destroying game" << std::endl;
   };

   void Game::fixedUpdate([[maybe_unused]] const Util::Timer& timer) {
      TracyMessageL("fixedUpdate");
      ZoneNamedN(upd, "FixedUpdate", true);
      Ecs::CameraSystem::fixedUpdate(*registry);
   }

   void Game::update() {
      TracyMessageL("render");
      Ecs::RenderSystem::update(*registry, *renderer);

      ImGui_ImplVulkan_NewFrame();
      ImGui_ImplGlfw_NewFrame();
      ImGui::NewFrame();

      entityEditor->renderSimpleCombo(*registry, room);

      renderer->render();
      FrameMark;
   }

   void Game::resize(const std::pair<uint32_t, uint32_t> size) {
      registry->ctx().insert_or_assign<Ecs::WindowDimensions>(
          Ecs::WindowDimensions{static_cast<int>(size.first), static_cast<int>(size.second)});
   }

   void Game::waitIdle() {
      renderer->waitIdle();
   }

   void Game::keyCallback(int key, int scancode, int action, int mods) {
      actionSystem->keyCallback(key, scancode, action, mods);
   }

   void Game::cursorPosCallback(double xpos, double ypos) {
      actionSystem->cursorPosCallback(xpos, ypos);
   }

   void Game::mouseButtonCallback(int button, int action, int mods) {
      actionSystem->mouseButtonCallback(button, action, mods);
   }

   void Game::setMouseState(bool captured) {
      actionSystem->setMouseState(captured);
   }
}