#pragma once

#include "cm/Handles.hpp"
#include "ozz/base/maths/soa_transform.h"

namespace tr::gp::ecs {
   struct Animation {
      Animation(cm::AnimationHandle animationHandle,
                cm::SkeletonHandle skeletonHandle,
                std::unordered_map<int, int> jointMap,
                std::vector<glm::mat4> inverseBindMatrices)
          : animationHandle{animationHandle},
            skeletonHandle{skeletonHandle},
            jointMap{jointMap},
            inverseBindMatrices{inverseBindMatrices} {
      }

      cm::AnimationHandle animationHandle;
      cm::SkeletonHandle skeletonHandle;

      bool renderBindPose{};
      bool playing{};
      std::string currentAnimationName;

      std::unordered_map<int, int> jointMap;
      std::vector<glm::mat4> inverseBindMatrices;

      float timeRatio{};
      ozz::animation::SamplingJob::Context context{};
      ozz::vector<ozz::math::SoaTransform> locals{};
      ozz::vector<ozz::math::Float4x4> models{};
   };
}

/*
   TODO: Think about fully abstracting away animations/skeletons.
   Either that or be ok with ozz::* stuff being in cmp::Animation and AnimationSystem
*/