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

         glm::mat4 matrix(-4.37114e-08,
                          0.0f,
                          1.0f,
                          0.0f, // First column
                          1.0f,
                          0.0f,
                          4.37114e-08,
                          0.0f, // Second column
                          0.0f,
                          1.0f,
                          0.0f,
                          0.0f, // Third column
                          0.0f,
                          0.0f,
                          0.0f,
                          1.0f // Fourth column
         );

         ozz::math::Float4x4 ozzMatrix;

         // Copy each element from glmMatrix to ozzMatrix
         for (int col = 0; col < 4; ++col) {
            for (int row = 0; row < 4; ++row) {
               ozzMatrix.cols[col][row] = matrix[col][row];
            }
         }

         // try adding the transformation matrix of cesium man's Z_UP node here
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
