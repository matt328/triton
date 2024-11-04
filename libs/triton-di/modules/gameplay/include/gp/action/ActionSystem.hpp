#pragma once

#include "IActionSystem.hpp"

namespace tr::gp {

   class ActionSystem : public IActionSystem {
    public:
      explicit ActionSystem(const std::shared_ptr<cm::evt::EventBus>& eventBus);
      virtual ~ActionSystem();

      void mapSource(Source source, cm::evt::StateType sType, cm::evt::ActionType aType) override;

    private:
      double prevX{}, prevY{};
      bool firstMouse = true;
      std::unordered_map<cm::Key, cm::evt::Action> keyActionMap;
      std::unordered_map<cm::MouseInput, cm::evt::Action> mouseActionMap;
   };

}
