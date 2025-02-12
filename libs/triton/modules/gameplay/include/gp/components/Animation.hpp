#pragma once
#include "cm/Handles.hpp"
#include "ozz/base/maths/soa_transform.h"

namespace tr {

struct Animation {
  Animation(const AnimationHandle animationHandle,
            const SkeletonHandle skeletonHandle,
            std::map<int, int> jointMap,
            std::vector<glm::mat4> inverseBindMatrices)
      : animationHandle{animationHandle},
        skeletonHandle{skeletonHandle},
        jointMap{std::move(jointMap)},
        inverseBindMatrices{std::move(inverseBindMatrices)} {
  }

  AnimationHandle animationHandle;
  SkeletonHandle skeletonHandle;

  bool renderBindPose{};
  bool playing{true};
  std::string currentAnimationName;

  std::map<int, int> jointMap;
  std::vector<glm::mat4> inverseBindMatrices;

  float timeRatio{};
  ozz::animation::SamplingJob::Context context{};
  ozz::vector<ozz::math::SoaTransform> locals{};
  ozz::vector<ozz::math::Float4x4> models{};
};

}
