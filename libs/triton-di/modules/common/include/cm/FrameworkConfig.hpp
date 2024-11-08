#pragma once

#include "event/EventBus.hpp"
#include "gp/GameplaySystem.hpp"
#include "gfx/VkGraphicsDevice.hpp"
#include "gp/action/ActionSystem.hpp"
#include "gfx/Renderer.hpp"
#include "di.hpp"
#include "cm/Timer.hpp"

namespace di = boost::di;

namespace tr::cm {
   static const auto config = gfx::VkGraphicsDevice::Config{
       .validationLayers = {"VK_LAYER_KHRONOS_validation"},
       .validationEnabled = true,
   };

   static const auto engineModule = [] {
      return di::make_injector(di::bind<gp::IGameplaySystem>.to<gp::GameplaySystem>(),
                               di::bind<gfx::IGraphicsDevice>.to<gfx::VkGraphicsDevice>(),
                               di::bind<gfx::VkGraphicsDevice::Config>.to(config),
                               di::bind<gfx::IRenderer>.to<gfx::Renderer>(),
                               di::bind<gp::IActionSystem>.to<gp::ActionSystem>(),
                               di::bind<cm::evt::IEventBus>.to<cm::evt::EventBus>());
   };

}
