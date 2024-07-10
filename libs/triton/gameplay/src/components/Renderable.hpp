#pragma once

#include "cm/Handles.hpp"

namespace tr::gp::cmp {
   struct Renderable {
      std::unordered_map<cm::MeshHandle, cm::TextureHandle> meshes;
   };
}
