#pragma once

#include "ActionSet.hpp"
namespace Game {
   class Scene {
    public:
      Scene(std::shared_ptr<Triton::Actions::ActionSet>& actionSet) : actionSet(actionSet) {
      }
      Scene(const Scene&) = default;
      Scene(Scene&&) = delete;
      Scene& operator=(const Scene&) = default;
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
      std::shared_ptr<Triton::Actions::ActionSet> actionSet;
   };

}