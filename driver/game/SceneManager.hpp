#pragma once

#include "ActionManager.hpp"

namespace Game {
   class Scene;

   class SceneManager {
    public:
      SceneManager() = default;

      template <typename T, typename... Args>
      size_t add(Args&&... args) {
         assert(actionManager != nullptr);
         auto actionSetId = actionManager->createActionSet();
         auto& actionSet = actionManager->getCurrentActionSet();

         scenes.emplace_back(std::make_unique<T>(actionSet, args...));
         return scenes.size() - 1;
      }

      void registeractionSet(std::shared_ptr<Triton::Actions::ActionManager> actionSet) {
         this->actionManager = actionSet;
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
