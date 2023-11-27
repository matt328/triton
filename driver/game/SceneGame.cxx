#include "SceneGame.hpp"

#include "ActionType.hpp"
#include "GameObject.hpp"
#include "Key.hpp"
#include "MeshComponent.hpp"
#include "Logger.hpp"

namespace Game {
   SceneGame::SceneGame(Triton::Actions::ActionSet& actionSet) : Scene(actionSet) {
      // Set up game objects
      auto gameObject = std::make_unique<GameObject>();
      gameObject->addComponent<MeshComponent>();
      gameObjects.push_back(std::move(gameObject));

      // Set up Scene Action Mappings
      actionSet.mapKey(Triton::Actions::Key::W, Triton::Actions::ActionType::MoveForward);
   }

   void SceneGame::onCreate() {
   }

   void SceneGame::onDestroy() {
   }
   void SceneGame::onActivate() {
   }
   void SceneGame::onDeactivate() {
   }
}