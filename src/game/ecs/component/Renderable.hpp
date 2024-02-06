#pragma once

namespace Triton::Game::Ecs {
   // HACK: eventually create Handle enum for these
   struct Renderable {
      uint32_t meshId;
      uint32_t textureId;
   };
}
