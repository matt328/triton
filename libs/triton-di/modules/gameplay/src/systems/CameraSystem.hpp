#pragma once

#include "gp/Registry.hpp"
#include "tr/IEventBus.hpp"
namespace tr {
   struct Action;
}

namespace tr::gp::sys {
   class CameraSystem {
    public:
      explicit CameraSystem(std::shared_ptr<IEventBus> newEventBus,
                            std::shared_ptr<Registry> newRegistry);
      ~CameraSystem();

      CameraSystem(const CameraSystem&) = default;
      CameraSystem(CameraSystem&&) = delete;
      auto operator=(const CameraSystem&) -> CameraSystem& = default;
      auto operator=(CameraSystem&&) -> CameraSystem& = delete;

      void fixedUpdate();

    private:
      std::shared_ptr<IEventBus> eventBus;
      std::shared_ptr<Registry> registry;
   };
}
