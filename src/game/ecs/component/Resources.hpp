#pragma once

namespace Triton::Game::Ecs {
   struct WindowDimensions {
      int width{}, height{};
   };

   struct CurrentCamera {
      entt::entity currentCamera;
   };
}