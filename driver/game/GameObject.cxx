#include "GameObject.hpp"

#include "Component.hpp"

namespace Game {
   void GameObject::update() const {
      for (const auto& component : allComponents) {
         component->update();
      }
   }
}