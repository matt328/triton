#pragma once

#include "gfx/Handles.hpp"

namespace tr::gp::ecs {
   struct Renderable {
      std::unordered_map<gfx::MeshHandle, gfx::TextureHandle> meshes;
   };
}
