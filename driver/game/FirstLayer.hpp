#pragma once

#include "Layer.hpp"

#include "GameObject.hpp"

namespace Game {
   class FirstLayer final : public Layer {

    public:
      explicit FirstLayer(Triton::Actions::ActionSet& actionSet);
      void onCreate() override;
      void onDestroy() override;
      void onActivate() override;
      void onDeactivate() override;

      void update() override;

    private:
      std::vector<std::unique_ptr<GameObject>> gameObjects;
   };
}
