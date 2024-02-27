#pragma once

#include "gfx/Handles.hpp"

namespace tr::gp::ecs {
   struct Renderable {
      gfx::MeshHandle meshId;
      gfx::TextureHandle textureId;
   };
}
