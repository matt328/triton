#pragma once

#include "gp/action/IActionSystem.hpp"
#include "tr/IEventBus.hpp"

namespace tr::gp {

   class ActionSystem : public IActionSystem {
    public:
      explicit ActionSystem(const std::shared_ptr<tr::IEventBus>& eventBus);
      virtual ~ActionSystem();

      void mapSource(Source source, tr::StateType sType, tr::ActionType aType) override;

    private:
      double prevX{}, prevY{};
      bool firstMouse = true;
      std::unordered_map<cm::Key, tr::Action> keyActionMap;
      std::unordered_map<cm::MouseInput, tr::Action> mouseActionMap;
   };

}
