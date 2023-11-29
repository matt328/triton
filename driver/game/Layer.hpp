#pragma once

#include "ActionSet.hpp"
#include "Events.hpp"

namespace Game {
   class Layer {
    public:
      explicit Layer(const std::shared_ptr<Triton::Actions::ActionSet>& actionSet)
         : actionSet(actionSet), active(false) {}

      Layer(const Layer&) = default;
      Layer(Layer&&) = delete;
      Layer& operator=(const Layer&) = delete;
      Layer& operator=(Layer&&) = delete;

      virtual ~Layer() = default;

      void onActivate() {
         active = true;
      };
      void onDeactivate() {
         active = false;
      };

      [[nodiscard]] const std::shared_ptr<Triton::Actions::ActionSet>& getActionSet() const {
         return actionSet;
      }

      virtual bool handleEvent(Triton::Events::Event& event) = 0;

   private:
      std::shared_ptr<Triton::Actions::ActionSet> actionSet;
      bool active;

   };

}