#pragma once

namespace tr::gp {
   struct Action;
   class EntitySystem;
}

namespace tr::gp::ecs::CameraSystem {
   void fixedUpdate(EntitySystem& entitySystem);

   void handleAction(EntitySystem& entitySystem, const Action& action);

}
