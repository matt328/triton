#pragma once

#include "gfx/Handles.hpp"
#include "ozz/base/maths/soa_transform.h"

namespace tr::gp::ecs {
   struct Animation {
      Animation(gfx::AnimationHandle animationHandle,
                gfx::SkeletonHandle skeletonHandle,
                const int numJoints,
                const int numSoaJoints)
          : animationHandle{animationHandle}, skeletonHandle{skeletonHandle} {
         context.Resize(numJoints);
         locals.resize(numSoaJoints, ozz::math::SoaTransform::identity());
         models.resize(numJoints, ozz::math::Float4x4::identity());
      }

      gfx::AnimationHandle animationHandle;
      gfx::SkeletonHandle skeletonHandle;

      bool renderBindPose{};
      bool playing{};
      std::string currentAnimationName;

      float timeRatio{};
      ozz::animation::SamplingJob::Context context{};
      ozz::vector<ozz::math::SoaTransform> locals{};
      ozz::vector<ozz::math::Float4x4> models{};
   };
}
