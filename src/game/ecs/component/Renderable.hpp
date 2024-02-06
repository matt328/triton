#pragma once

namespace Triton::Game::Ecs {
   // HACK: eventually create Handle enum for these
   struct Renderable {
      std::string meshId;
      uint32_t textureId;
   };
}
