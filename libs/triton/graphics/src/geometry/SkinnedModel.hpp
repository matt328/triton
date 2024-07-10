#pragma once

#include "cm/Handles.hpp"

namespace tr::gfx::geo {
   struct SkinnedModel {
      LoadedSkinnedModelData modelHandle;
      ozz::animation::Skeleton skeleton;
      ozz::animation::Animation animation;
   };
}
