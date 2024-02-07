#pragma once

#include "graphics/Handles.hpp"

namespace Triton::Game::Ecs {
   // HACK: eventually create Handle enum for these
   struct Renderable {
      Graphics::MeshHandle meshId;
      uint32_t textureId;
   };
}
