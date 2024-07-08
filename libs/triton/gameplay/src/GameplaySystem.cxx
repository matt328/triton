#include "gp/GameplaySystem.hpp"

#include "cm/Inputs.hpp"

#include "EntitySystem.hpp"

#include "actions/ActionType.hpp"
#include "actions/ActionSystem.hpp"
#include "actions/Action.hpp"
#include "actions/Sources.hpp"

#include "systems/CameraSystem.hpp"
#include "systems/RenderDataSystem.hpp"
#include "systems/AnimationSystem.hpp"

namespace tr::gp {

   /*
      TODO: Either make AnimationSystem a class with a reference to the AnimationFactory
      or make this class hold a reference to the AnimationFactory and pass it into
      AnimationSystem::update()
   */

   using namespace tr::gp;

   GameplaySystem::GameplaySystem(gfx::geo::AnimationFactory& animationFactory)
       : animationFactory{animationFactory} {

      entitySystem = std::make_unique<EntitySystem>();
      actionSystem = std::make_unique<ActionSystem>();

      using gp::Key;

      // TODO: synchronization isn't applied correctly (at all) here.
      actionSystem->getDelegate().connect<&ecs::CameraSystem::handleAction>(
          *entitySystem->getRegistry());

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

   void GameplaySystem::fixedUpdate([[maybe_unused]] const cm::Timer& timer) {
      ZoneNamedN(upd, "FixedUpdate", true);
      entitySystem->fixedUpdate(timer, animationFactory);
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
      renderData.skinnedMeshData.clear();
      renderData.animationData.clear();

      entitySystem->prepareRenderData(renderData);

      renderDataProducer(renderData);
   }

   void GameplaySystem::resize(const std::pair<uint32_t, uint32_t> size) {
      entitySystem->writeWindowDimensions(size);
   }

   void GameplaySystem::keyCallback(gp::Key key, gp::ButtonState buttonState) {
      actionSystem->keyCallback(key, buttonState);
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