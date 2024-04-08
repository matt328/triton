#pragma once

namespace tr::gfx {
   using TextureHandle = uint32_t;
   using MeshHandle = size_t;
   using MaterialHandle = uint32_t;
   using ModelInfoHandle = uint32_t;
   using MeshMaterialHandle = std::unordered_map<MaterialHandle, std::vector<MeshHandle>>;
   using ModelHandle = std::unordered_map<MeshHandle, TextureHandle>;
}