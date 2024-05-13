#pragma once

namespace tr::gfx::geo {

   using GeometryHandle = size_t;
   using ImageHandle = size_t;
   using TexturedGeometryHandle = std::unordered_map<GeometryHandle, ImageHandle>;

   struct SkinnedGeometryData {
      GeometryHandle geometryHandle;
      ImageHandle imageHandle;
      ozz::animation::Skeleton skeleton{};
      ozz::animation::Animation animation{};
   };

}
