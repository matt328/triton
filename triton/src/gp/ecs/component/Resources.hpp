#pragma once

namespace tr::gp::ecs {
   struct WindowDimensions {
      int width{}, height{};
   };

   struct CurrentCamera {
      entt::entity currentCamera;
   };
}