#pragma once

#include "ActionSet.hpp"
namespace Game {
   class Scene {
    public:
      Scene(Triton::Actions::ActionSet& actionSet) : actionSet(actionSet) {
      }
      Scene(const Scene&) = default;
      Scene(Scene&&) = delete;
      Scene& operator=(const Scene&) = delete;
      Scene& operator=(Scene&&) = delete;

      virtual ~Scene() = default;

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