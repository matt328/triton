#pragma once

#include "graphics/Handles.hpp"

namespace Triton::Game::Ecs {
   struct Renderable {
      Graphics::MeshHandle meshId;
      Graphics::TextureHandle textureId;
   };
}
