#pragma once

#include "cm/Handles.hpp"

namespace tr::gp::ecs {
   struct Renderable {
      std::unordered_map<cm::MeshHandle, cm::TextureHandle> meshes;
   };
}
