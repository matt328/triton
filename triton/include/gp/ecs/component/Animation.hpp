#pragma once

namespace tr::gp::ecs {
   struct Animation {
      float timeRatio{};
      ozz::animation::SamplingJob::Context context;
      ozz::animation::Skeleton skeleton{};
      ozz::animation::Animation animation{};
      ozz::vector<ozz::math::SoaTransform> locals{};
      ozz::vector<ozz::math::Float4x4> models{};
   };
}