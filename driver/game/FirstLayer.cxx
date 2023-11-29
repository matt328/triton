#include "FirstLayer.hpp"

#include "ActionType.hpp"
#include "GameObject.hpp"
#include "Key.hpp"
#include "MeshComponent.hpp"

namespace Game {
   FirstLayer::FirstLayer(Triton::Actions::ActionSet& actionSet) : Layer(actionSet) {
      // Set up game objects
      auto gameObject = std::make_unique<GameObject>();
      gameObject->addComponent<MeshComponent>();
      gameObjects.push_back(std::move(gameObject));

      // Set up Layer Action Mappings
      actionSet.mapKey(Triton::Actions::Key::W, Triton::Actions::ActionType::MoveForward);
   }

   void FirstLayer::update() {

   }


   void FirstLayer::onCreate() {
   }

   void FirstLayer::onDestroy() {
   }
   void FirstLayer::onActivate() {
   }
   void FirstLayer::onDeactivate() {
   }
}