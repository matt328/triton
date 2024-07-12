#pragma once

#include "cm/Handles.hpp"

namespace tr::gfx::geo {

   using GeometryHandle = size_t;
   using ImageHandle = size_t;
   using TexturedGeometryHandle = std::unordered_map<GeometryHandle, ImageHandle>;

   struct SkinnedGeometryData {
      GeometryHandle geometryHandle;
      ImageHandle imageHandle;
      cm::SkeletonHandle skeletonHandle;
      cm::AnimationHandle animationHandle;
      std::unordered_map<int, int> jointMap;
      std::vector<glm::mat4> inverseBindMatrices;
   };

}
