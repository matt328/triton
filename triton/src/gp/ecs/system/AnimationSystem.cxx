#include "gfx/geometry/AnimationFactory.hpp"
#include "gp/ecs/component/Animation.hpp"
#include "ozz/base/maths/simd_math.h"

namespace tr::gp::ecs::AnimationSystem {
   void update(entt::registry& registry, gfx::geo::AnimationFactory& animationFactory) {
      const auto view = registry.view<Animation>();

      for (auto [entity, animationData] : view.each()) {
         const auto& animation = animationFactory.getAnimation(animationData.animationHandle);
         float dt = 1.f / 120.f;
         auto newTime = animationData.timeRatio + dt * (1.f / animation.duration());

         if (newTime > 1.f) {
            newTime = 0.f;
         }

         auto samplingJob = ozz::animation::SamplingJob{};
         samplingJob.animation = &animation;
         samplingJob.context = &animationData.context;
         samplingJob.ratio = newTime;
         samplingJob.output = ozz::make_span(animationData.locals);
         if (!samplingJob.Run()) {
            Log::warn << "Sampling job fail" << std::endl;
         }

         alignas(16) float temp[4];
         temp[0] = 0.f;
         temp[1] = 0.f;
         temp[2] = 0.f;
         temp[3] = 0.f;

         alignas(16) float rotation[4];
         rotation[0] = 0.7071068286895752f;
         rotation[1] = 0.f;
         rotation[2] = 0.f;
         rotation[3] = 0.7071068286895752f;

         alignas(16) float scale[4];
         scale[0] = 0.009999999776482582f;
         scale[1] = 0.009999999776482582f;
         scale[2] = 0.009999999776482582f;
         scale[3] = 0.f;

         const ozz::math::SimdFloat4 t = ozz::math::simd_float4::Load3PtrU(temp);
         const ozz::math::SimdFloat4 r = ozz::math::simd_float4::LoadPtrU(rotation);
         const ozz::math::SimdFloat4 s = ozz::math::simd_float4::Load3PtrU(scale);
         const ozz::math::Float4x4 rootMatrix = ozz::math::Float4x4::FromAffine(t, r, s);

         const auto& skeleton = animationFactory.getSkeleton(animationData.skeletonHandle);
         auto ltmJob = ozz::animation::LocalToModelJob{};
         ltmJob.skeleton = &skeleton;
         ltmJob.root = &rootMatrix;
         ltmJob.input = ozz::make_span(animationData.locals);
         ltmJob.output = ozz::make_span(animationData.models);
         if (!ltmJob.Run()) {
            Log::warn << "Ltm Job Fail" << std::endl;
         }

         animationData.timeRatio = newTime;
      }
   }
}
