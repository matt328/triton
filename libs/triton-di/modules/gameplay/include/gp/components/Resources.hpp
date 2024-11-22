#pragma once

namespace tr::gp::cmp {
   struct WindowDimensions {
      uint32_t width{}, height{};
   };

   struct CurrentCamera {
      entt::entity currentCamera;
   };
}