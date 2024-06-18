#pragma once

#include "Model.hpp"

namespace al {
   /// This represents the output of converting an fbx file
   struct ModelData {
      Model model;
      ozz::animation::Skeleton skeleton;
      ozz::animation::Animation animation;
   };
}
