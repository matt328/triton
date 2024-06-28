#pragma once

#include "Model.hpp"

namespace al {
   /// This represents triton's internal model format.
   /// Probably won't end up being used.
   struct ModelData {
      Model model;
      ozz::animation::Skeleton skeleton;
      ozz::animation::Animation animation;
   };
}
