#include "GameplaySystem.hpp"

namespace tr::gp {

   GameplaySystem::GameplaySystem(const std::shared_ptr<IActionSystem>& actionSystem) {
      Log.debug("Creating Gameplay System");

      using cm::Key;
      namespace evt = cm::evt;

      // TODO(matt): synchronization isn't applied correctly (at all) here.
      // actionSystem->getDelegate().connect<&sys::CameraSystem::handleAction>(
      //     *behaviorSystem->getRegistry());

      // Forward
      actionSystem->mapSource(Source{cm::Key::Up, SourceType::Boolean},
                              evt::StateType::State,
                              evt::ActionType::MoveForward);
      actionSystem->mapSource(Source{cm::Key::W, SourceType::Boolean},
                              evt::StateType::State,
                              evt::ActionType::MoveForward);

      // Backward
      actionSystem->mapSource(Source{cm::Key::Down, SourceType::Boolean},
                              evt::StateType::State,
                              evt::ActionType::MoveBackward);
      actionSystem->mapSource(Source{cm::Key::S, SourceType::Boolean},
                              evt::StateType::State,
                              evt::ActionType::MoveBackward);
      // Left
      actionSystem->mapSource(Source{cm::Key::Left, SourceType::Boolean},
                              evt::StateType::State,
                              evt::ActionType::StrafeLeft);
      actionSystem->mapSource(Source{cm::Key::A, SourceType::Boolean},
                              evt::StateType::State,
                              evt::ActionType::StrafeLeft);
      // Right
      actionSystem->mapSource(Source{cm::Key::Right, SourceType::Boolean},
                              evt::StateType::State,
                              evt::ActionType::StrafeRight);
      actionSystem->mapSource(Source{cm::Key::D, SourceType::Boolean},
                              evt::StateType::State,
                              evt::ActionType::StrafeRight);
      // Look
      actionSystem->mapSource(Source{cm::MouseInput::MOVE_X, SourceType::Float},
                              evt::StateType::Range,
                              evt::ActionType::LookHorizontal);
      actionSystem->mapSource(Source{cm::MouseInput::MOVE_Y, SourceType::Float},
                              evt::StateType::Range,
                              evt::ActionType::LookVertical);
   }

   void GameplaySystem::update() {
   }

   void GameplaySystem::fixedUpdate() {
   }
}