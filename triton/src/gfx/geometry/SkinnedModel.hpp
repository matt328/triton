#pragma once

#include "gfx/Handles.hpp"

namespace tr::gfx::geo {
   struct SkinnedModel {
      SkinnedModelHandle modelHandle;
      ozz::animation::Skeleton skeleton;
      ozz::animation::Animation animation;
   };
}
