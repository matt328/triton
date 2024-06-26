#include "gfx/geometry/AnimationFactory.hpp"
#include "gp/ecs/component/Animation.hpp"

namespace tr::gp::ecs::AnimationSystem {
   void update(entt::registry& registry, gfx::geo::AnimationFactory& animationFactory) {
      const auto view = registry.view<Animation>();

      for (auto [entity, animationData] : view.each()) {

         if (animationData.renderBindPose) {
            for (auto& model : animationData.models) {
               model = ozz::math::Float4x4::identity();
            }
            continue;
         }

         if (animationData.playing) {
            animationData.timeRatio += 0.005f;
            if (animationData.timeRatio >= 1.f) {
               animationData.timeRatio = 0.f;
            }
         }

         const auto& animation = animationFactory.getAnimation(animationData.animationHandle);

         auto samplingJob = ozz::animation::SamplingJob{};
         samplingJob.animation = &animation;
         samplingJob.context = &animationData.context;
         samplingJob.ratio = animationData.timeRatio;
         samplingJob.output = ozz::make_span(animationData.locals);
         if (!samplingJob.Run()) {
            Log::warn << "Sampling job fail" << std::endl;
         }

         const auto& skeleton = animationFactory.getSkeleton(animationData.skeletonHandle);
         auto ltmJob = ozz::animation::LocalToModelJob{};
         ltmJob.skeleton = &skeleton;
         ltmJob.input = ozz::make_span(animationData.locals);
         ltmJob.output = ozz::make_span(animationData.models);
         if (!ltmJob.Run()) {
            Log::warn << "Ltm Job Fail" << std::endl;
         }
      }
   }
}
