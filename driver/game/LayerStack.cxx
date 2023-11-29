#include "LayerStack.hpp"
#include "cassert"
#include "Layer.hpp"
#include "Logger.hpp"

namespace Game {

   void LayerStack::switchTo(const size_t id) {
      assert(currentLayer < layerStack.size());

      layerStack[currentLayer]->onDeactivate();

      currentLayer = id;

      layerStack[currentLayer]->onActivate();

      actionManager->setCurrentActionSet(layerStack[id]->getActionSet());
   }

   void LayerStack::remove(const size_t id) {
      assert(id != currentLayer);

      if (id < layerStack.size()) {
         layerStack.erase(layerStack.begin() + id);
      }
   }
   void LayerStack::handleEvent(Triton::Events::Event& event) {
      for(const auto& layer : this->layerStack) {
         if(layer->handleEvent(event)) {
            break;
         }
      }
   }

}