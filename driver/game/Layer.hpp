#pragma once

#include "ActionSet.hpp"
namespace Game {
   class Layer {
    public:
      explicit Layer(Triton::Actions::ActionSet& actionSet) : actionSet(actionSet) {
      }
      Layer(const Layer&) = default;
      Layer(Layer&&) = delete;
      Layer& operator=(const Layer&) = delete;
      Layer& operator=(Layer&&) = delete;

      virtual ~Layer() = default;

      virtual void onCreate() = 0;
      virtual void onDestroy() = 0;
      virtual void onActivate() = 0;
      virtual void onDeactivate() = 0;

      virtual void processInput(){};
      virtual void update(){};
      virtual void draw(){};

    private:
      Triton::Actions::ActionSet& actionSet;
   };

}