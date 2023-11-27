#pragma once

#include "Scene.hpp"

#include "GameObject.hpp"

namespace Game {

   //    class GameObject;

   class SceneGame : public Scene {

    public:
      SceneGame(Triton::Actions::ActionSet& actionSet);
      void onCreate() override;
      void onDestroy() override;
      void onActivate() override;
      void onDeactivate() override;

      void update() override;

    private:
      std::vector<std::unique_ptr<GameObject>> gameObjects;
   };
}
