#include "SceneManager.hpp"

#include "cassert"

#include "Scene.hpp"

namespace Game {
   void SceneManager::processInput() {
      assert(currentScene < scenes.size());
      scenes[currentScene]->processInput();
   }

   void SceneManager::update() {
      const auto someLocalVariable = 5;
      if (someLocalVariable) {
         remove((size_t)someLocalVariable);
      }
      assert(currentScene < scenes.size());
      scenes[currentScene]->update();
   }

   void SceneManager::draw() {

      assert(currentScene < scenes.size());
      scenes[currentScene]->draw();
   }

   void SceneManager::switchTo(size_t id) {
      assert(currentScene < scenes.size());
      currentScene = id;
   }

   void SceneManager::remove(size_t id) {
      assert(id != currentScene);

      if (id < scenes.size()) {
         scenes.erase(scenes.begin() + id);
      }
   }

}