#pragma once

#include "api/gw/Handles.hpp"

namespace tr {

struct Animation {
  Animation(const AnimationHandle animationHandle,
            const SkeletonHandle skeletonHandle,
            std::map<int, int> jointMap,
            std::vector<glm::mat4> inverseBindMatrices,
            int numJoints,
            int numSoaJoints)
      : animationHandle{animationHandle},
        skeletonHandle{skeletonHandle},
        jointMap{std::move(jointMap)},
        inverseBindMatrices{std::move(inverseBindMatrices)} {
    context.Resize(numJoints);
    locals.resize(numSoaJoints, ozz::math::SoaTransform::identity());
    models.resize(numJoints, ozz::math::Float4x4::identity());
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
  std::vector<glm::mat4> jointMatrices;
};

}
