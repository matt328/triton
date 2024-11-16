#include "GameplaySystem.hpp"
#include "cm/RenderData.hpp"
#include "tr/IGameplaySystem.hpp"

namespace tr::gp {

   GameplaySystem::GameplaySystem(const std::shared_ptr<IActionSystem>& actionSystem) {
      Log.debug("Creating Gameplay System");

      // TODO(matt): synchronization isn't applied correctly (at all) here.
      // actionSystem->getDelegate().connect<&sys::CameraSystem::handleAction>(
      //     *behaviorSystem->getRegistry());

      // Forward
      actionSystem->mapSource(Source{cm::Key::Up, SourceType::Boolean},
                              tr::StateType::State,
                              tr::ActionType::MoveForward);
      actionSystem->mapSource(Source{cm::Key::W, SourceType::Boolean},
                              tr::StateType::State,
                              tr::ActionType::MoveForward);

      // Backward
      actionSystem->mapSource(Source{cm::Key::Down, SourceType::Boolean},
                              tr::StateType::State,
                              tr::ActionType::MoveBackward);
      actionSystem->mapSource(Source{cm::Key::S, SourceType::Boolean},
                              tr::StateType::State,
                              tr::ActionType::MoveBackward);
      // Left
      actionSystem->mapSource(Source{cm::Key::Left, SourceType::Boolean},
                              tr::StateType::State,
                              tr::ActionType::StrafeLeft);
      actionSystem->mapSource(Source{cm::Key::A, SourceType::Boolean},
                              tr::StateType::State,
                              tr::ActionType::StrafeLeft);
      // Right
      actionSystem->mapSource(Source{cm::Key::Right, SourceType::Boolean},
                              tr::StateType::State,
                              tr::ActionType::StrafeRight);
      actionSystem->mapSource(Source{cm::Key::D, SourceType::Boolean},
                              tr::StateType::State,
                              tr::ActionType::StrafeRight);
      // Look
      actionSystem->mapSource(Source{cm::MouseInput::MOVE_X, SourceType::Float},
                              tr::StateType::Range,
                              tr::ActionType::LookHorizontal);
      actionSystem->mapSource(Source{cm::MouseInput::MOVE_Y, SourceType::Float},
                              tr::StateType::Range,
                              tr::ActionType::LookVertical);
   }

   void GameplaySystem::update() {
   }

   void GameplaySystem::fixedUpdate() {
   }

   void GameplaySystem::setRenderDataTransferHandler(const RenderDataTransferHandler& handler) {
      this->transferHandler = handler;
   }
}