#pragma once

namespace tr::gp {
   struct Action;
}

namespace tr::gp::sys::CameraSystem {
   void fixedUpdate(entt::registry& registry);

   void handleAction(entt::registry& registry, const Action& action);

}
