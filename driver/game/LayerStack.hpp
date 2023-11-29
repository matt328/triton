#pragma once

#include "ActionManager.hpp"
#include "Logger.hpp"

namespace Game {
   class Layer;

   class LayerStack {
    public:
      LayerStack() = default;

      template <typename T, typename... Args>
      size_t pushNew(Args&&... args) {
         assert(actionManager != nullptr);
         const auto actionSetId = actionManager->createActionSet();

         Log::debug << "Action Set Id: " << actionSetId << std::endl;

         auto& actionSet = actionManager->getCurrentActionSet();

         scenes.emplace_back(std::make_unique<T>(actionSet, args...));
         return scenes.size() - 1;
      }

      void registeractionSet(const std::shared_ptr<Triton::Actions::ActionManager>& actionSet) {
         this->actionManager = actionSet;
      }

      void switchTo(size_t id);
      void remove(size_t id);
      void processInput();
      void update();
      void draw();

    private:
      size_t currentScene{};
      std::vector<std::unique_ptr<Layer>> scenes;

      std::shared_ptr<Triton::Actions::ActionManager> actionManager;
   };
}
