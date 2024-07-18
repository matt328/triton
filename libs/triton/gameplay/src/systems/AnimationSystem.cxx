#include "components/Animation.hpp"

#include "AnimationFactory.hpp"

namespace tr::gp::sys::AnimationSystem {
   void update(entt::registry& registry, AnimationFactory& animationFactory) {
      const auto view = registry.view<cmp::Animation>();

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
         const auto& skeleton = animationFactory.getSkeleton(animationData.animationHandle);

         // This should do this once, then be a no op on subsequent invocations
         animationData.context.Resize(skeleton.num_joints());
         animationData.locals.resize(skeleton.num_soa_joints(),
                                     ozz::math::SoaTransform::identity());
         animationData.models.resize(skeleton.num_joints(), ozz::math::Float4x4::identity());

         auto samplingJob = ozz::animation::SamplingJob{};
         samplingJob.animation = &animation;
         samplingJob.context = &animationData.context;
         samplingJob.ratio = animationData.timeRatio;
         samplingJob.output = ozz::make_span(animationData.locals);
         if (!samplingJob.Run()) {
            Log.warn("Sampling job fail");
         }

         auto ltmJob = ozz::animation::LocalToModelJob{};
         ltmJob.skeleton = &skeleton;
         ltmJob.input = ozz::make_span(animationData.locals);
         ltmJob.output = ozz::make_span(animationData.models);
         if (!ltmJob.Run()) {
            Log.warn("ltm job fail");
         }
      }
   }
}
