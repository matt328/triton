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

         /* TODO: The order of these matrices is not matching the order of the joints in the gltf
          file? Either switch to fbx or implement gltf animation myself so I fully understand what
          is happening.
          I'm not seeing how the order of the joints matters since they aren't a thing when
          loading the gltf's mesh, i'm ignoring them.
          The joints in the gltf file contain just a transformation, and these JointIds exist in the
          vertex data, so the order of the matrices here has to match the order of how the joints
          are defined in the gltf file. When loading the gltf file, we need to map the index number
          of the joint to the joint name then consult that map to order the model matrices

          When loading the gltf, log each joint name
          When
          */

         animationData.timeRatio = newTime;
      }
   }
}
