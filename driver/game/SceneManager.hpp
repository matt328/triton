#pragma once

#include "ActionSet.hpp"

namespace Game {
   class Scene;

   class SceneManager {
    public:
      SceneManager() = default;

      template <typename T, typename... Args>
      size_t add(Args&&... args) {
         scenes.emplace_back(std::make_unique<T>(this->actionSet, args...));
         return scenes.size() - 1;
      }

      void registeractionSet(std::shared_ptr<Triton::Actions::ActionSet> actionSet) {
         this->actionSet = actionSet;
      }

      void switchTo(size_t id);
      void remove(size_t id);
      void processInput();
      void update();
      void draw();

    private:
      size_t currentScene{};
      std::vector<std::unique_ptr<Scene>> scenes;

      std::shared_ptr<Triton::Actions::ActionSet> actionSet;
   };
}
