#pragma once

#include "Layer.hpp"
#include "GameObject.hpp"
#include "Events.hpp"

namespace Game {
   class FirstLayer final : public Layer {

    public:
      explicit FirstLayer(const std::shared_ptr<Triton::Actions::ActionSet>& actionSet);

      bool handleEvent(Triton::Events::Event& event) override;

      [[nodiscard]] bool handleAction(Triton::Actions::ActionType action) const;

    private:
      std::vector<std::unique_ptr<GameObject>> gameObjects;
   };
}
