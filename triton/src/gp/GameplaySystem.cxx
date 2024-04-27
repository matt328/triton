#include "gp/GameplaySystem.hpp"

#include "gp/actions/ActionType.hpp"
#include "gp/actions/ActionSystem.hpp"
#include "gp/actions/Inputs.hpp"
#include "gp/actions/Action.hpp"
#include "gp/actions/Sources.hpp"

#include "gp/ecs/component/Resources.hpp"
#include "gp/ecs/system/CameraSystem.hpp"
#include "gp/ecs/system/RenderDataSystem.hpp"
#include "gp/ecs/system/TransformSystem.hpp"
#include "gp/ecs/component/Resources.hpp"

namespace tr::gp {

   using namespace tr::gp;

   GameplaySystem::GameplaySystem() : registry{std::make_unique<entt::registry>()} {
      actionSystem = std::make_unique<ActionSystem>();

      auto& reg = *registry;

      actionSystem->getDelegate().connect<&ecs::CameraSystem::handleAction>(reg);

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
   }

   GameplaySystem::~GameplaySystem() {
      Log::info << "destroying game" << std::endl;
   };

   void GameplaySystem::fixedUpdate([[maybe_unused]] const util::Timer& timer) {
      ZoneNamedN(upd, "FixedUpdate", true);

      ecs::CameraSystem::fixedUpdate(*registry);
      ecs::TransformSystem::update(*registry);
   }

   void GameplaySystem::update(const double blendingFactor) {
      TracyPlot("Physics Blend Factor", blendingFactor);
      ZoneNamedN(upd, "Update", true);

      // TODO: implement some logic to move things around
      // and if it looks unsmooth or sus or whatever, then add interpolation based on blending
      // factor. This probably won't factor in until we have physics and skeletal animation
      renderData.objectData.clear();
      renderData.staticMeshData.clear();
      renderData.terrainMeshData.clear();

      ecs::RenderDataSystem::update(*registry, renderData);

      renderDataProducer(renderData);
   }

   void GameplaySystem::resize(const std::pair<uint32_t, uint32_t> size) {
      registry->ctx().insert_or_assign<ecs::WindowDimensions>(
          ecs::WindowDimensions{static_cast<int>(size.first), static_cast<int>(size.second)});
   }

   void GameplaySystem::keyCallback(int key, int scancode, int action, int mods) {
      actionSystem->keyCallback(key, scancode, action, mods);
   }

   void GameplaySystem::cursorPosCallback(double xpos, double ypos) {
      actionSystem->cursorPosCallback(xpos, ypos);
   }

   void GameplaySystem::mouseButtonCallback(int button, int action, int mods) {
      actionSystem->mouseButtonCallback(button, action, mods);
   }

   void GameplaySystem::setMouseState(bool captured) {
      actionSystem->setMouseState(captured);
   }
}