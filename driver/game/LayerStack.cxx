#include "LayerStack.hpp"

#include "cassert"

#include "Layer.hpp"

namespace Game {
   void LayerStack::processInput() {
      assert(currentScene < scenes.size());
      scenes[currentScene]->processInput();
   }

   void LayerStack::update() {
      assert(currentScene < scenes.size());
      scenes[currentScene]->update();
   }

   void LayerStack::draw() {
      assert(currentScene < scenes.size());
      scenes[currentScene]->draw();
   }

   void LayerStack::switchTo(size_t id) {
      assert(currentScene < scenes.size());
      currentScene = id;
   }

   void LayerStack::remove(size_t id) {
      assert(id != currentScene);

      if (id < scenes.size()) {
         scenes.erase(scenes.begin() + id);
      }
   }

}