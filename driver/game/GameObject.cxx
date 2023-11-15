#include "GameObject.hpp"

#include "Component.hpp"

namespace Game {
   void GameObject::update() {
      for (auto& component : allComponents) {
         component->update();
      }
   }
}