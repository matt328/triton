#include "SceneGame.hpp"

#include "GameObject.hpp"
#include "MeshComponent.hpp"

namespace Game {
   SceneGame::SceneGame(std::shared_ptr<Triton::Actions::ActionManager>& actionManager) :
       Scene(actionManager) {
      auto gameObject = std::make_unique<GameObject>();
      gameObject->addComponent<MeshComponent>();
      gameObjects.push_back(std::move(gameObject));
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