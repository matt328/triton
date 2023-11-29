#include "FirstLayer.hpp"

#include "ActionType.hpp"
#include "GameObject.hpp"
#include "Key.hpp"
#include "MeshComponent.hpp"
#include "Logger.hpp"

#include <events/ActionEvent.hpp>

namespace Game {
   FirstLayer::FirstLayer(const std::shared_ptr<Triton::Actions::ActionSet>& actionSet) :
       Layer(actionSet) {
      auto gameObject = std::make_unique<GameObject>();
      gameObject->addComponent<MeshComponent>();
      gameObjects.push_back(std::move(gameObject));

      // Map Actions
      actionSet->mapKey(Triton::Actions::Key::A, Triton::Actions::ActionType::StrafeLeft);
   }

   bool FirstLayer::handleEvent(Triton::Events::Event& event) {
      auto dispatcher = Triton::Events::EventDispatcher{event};
      return dispatcher.dispatch<Triton::Events::ActionEvent>([this](Triton::Events::ActionEvent& e) {
         return handleAction(e.getActionType());
      });
   }

   bool FirstLayer::handleAction(const Triton::Actions::ActionType action) const{
      Log::debug << "First Layer recieved action: " << action << std::endl;
      return true;
   }

}