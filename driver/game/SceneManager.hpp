#pragma once

#include "ActionManager.hpp"
namespace Game {
   class Scene;

   class SceneManager {
    public:
      SceneManager() = default;

      template <typename T, typename... Args>
      size_t add(Args&&... args) {
         scenes.emplace_back(std::make_unique<T>(this->actionManager, args...));
         return scenes.size() - 1;
      }

      void registerActionManager(std::shared_ptr<Triton::Actions::ActionManager> actionManager) {
         this->actionManager = actionManager;
      }

      void switchTo(size_t id);
      void remove(size_t id);
      void processInput();
      void update();
      void draw();

    private:
      size_t currentScene{};
      std::vector<std::unique_ptr<Scene>> scenes;

      std::shared_ptr<Triton::Actions::ActionManager> actionManager;
   };
}
