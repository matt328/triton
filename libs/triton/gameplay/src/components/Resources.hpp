#pragma once

namespace tr::gp::cmp {
   struct WindowDimensions {
      int width{}, height{};
   };

   struct CurrentCamera {
      entt::entity currentCamera;
   };
}