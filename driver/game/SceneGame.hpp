#pragma once

#include "Scene.hpp"

namespace Game {

   class GameObject;

   class SceneGame : public Scene {

    public:
      SceneGame(std::shared_ptr<Triton::Actions::ActionManager>& actionManager);
      void onCreate() override;
      void onDestroy() override;
      void onActivate() override;
      void onDeactivate() override;

    private:
      std::vector<std::unique_ptr<GameObject>> gameObjects;
   };
}