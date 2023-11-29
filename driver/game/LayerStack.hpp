#pragma once

#include "ActionManager.hpp"

#include <Events.hpp>

namespace Game {
   class Layer;

   class LayerStack {
    public:
      explicit LayerStack(const std::shared_ptr<Triton::Actions::ActionManager>& actionManager)
         : actionManager(actionManager) {}

      template <typename T, typename... Args>
      size_t pushNew(Args&&... args) {
         auto actionSet = std::make_shared<Triton::Actions::ActionSet>();
         layerStack.emplace_back(std::make_unique<T>(actionSet, args...));
         return layerStack.size() - 1;
      }

      void switchTo(size_t id);
      void remove(size_t id);

      void handleEvent(Triton::Events::Event& event);

    private:
      size_t currentLayer{};
      std::vector<std::unique_ptr<Layer>> layerStack;
      std::shared_ptr<Triton::Actions::ActionManager> actionManager;
   };
}
