#pragma once

#include "gfx/Renderer.hpp"
#include "gp/GameplaySystem.hpp"
#include "di.hpp"

namespace di = boost::di;

#include "cm/event/EventBus.hpp"
#include "cm/Timer.hpp"

namespace tr::cm {
   static const auto engineModule = [] {
      return di::make_injector(di::bind<gp::IGameplaySystem>.to<gp::GameplaySystem>(),
                               di::bind<gfx::IRenderer>.to<gfx::Renderer>());
   };
}
