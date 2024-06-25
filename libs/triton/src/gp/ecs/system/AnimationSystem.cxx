#include "gfx/geometry/AnimationFactory.hpp"
#include "gp/ecs/component/Animation.hpp"

namespace tr::gp::ecs::AnimationSystem {
   void update(entt::registry& registry, gfx::geo::AnimationFactory& animationFactory) {
      const auto view = registry.view<Animation>();

      for (auto [entity, animationData] : view.each()) {
         const auto& animation = animationFactory.getAnimation(animationData.animationHandle);

         auto samplingJob = ozz::animation::SamplingJob{};
         samplingJob.animation = &animation;
         samplingJob.context = &animationData.context;
         samplingJob.ratio = animationData.timeRatio;
         samplingJob.output = ozz::make_span(animationData.locals);
         if (!samplingJob.Run()) {
            Log::warn << "Sampling job fail" << std::endl;
         }

         auto mat = glm::identity<glm::mat4>();
         mat = glm::scale(mat, glm::vec3(1.f, 1.f, -1.f));
         auto angle = glm::radians(180.f);
         auto yaxis = glm::vec3(0.f, 1.f, 0.f);
         mat = glm::rotate(mat, angle, yaxis);

         ozz::math::Float4x4 ozzMatrix;

         // Copy each element from glmMatrix to ozzMatrix
         for (int col = 0; col < 4; ++col) {
            for (int row = 0; row < 4; ++row) {
               ozzMatrix.cols[col][row] = mat[col][row];
            }
         }

         const auto& skeleton = animationFactory.getSkeleton(animationData.skeletonHandle);
         auto ltmJob = ozz::animation::LocalToModelJob{};
         ltmJob.skeleton = &skeleton;
         ltmJob.root = nullptr;
         ltmJob.input = ozz::make_span(animationData.locals);
         ltmJob.output = ozz::make_span(animationData.models);
         if (!ltmJob.Run()) {
            Log::warn << "Ltm Job Fail" << std::endl;
         }
      }
   }
}
