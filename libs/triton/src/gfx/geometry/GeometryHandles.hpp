#pragma once

#include "gfx/geometry/AnimationFactory.hpp"
namespace tr::gfx::geo {

   using GeometryHandle = size_t;
   using ImageHandle = size_t;
   using TexturedGeometryHandle = std::unordered_map<GeometryHandle, ImageHandle>;

   struct SkinnedGeometryData {
      GeometryHandle geometryHandle;
      ImageHandle imageHandle;
      SkeletonHandle skeletonHandle;
      AnimationHandle animationHandle;
      std::unordered_map<int, int> jointMap;
      std::vector<glm::mat4> inverseBindMatrices;
   };

}
