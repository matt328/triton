#include "AnimationSystem.hpp"

#include "gp/ecs/component/Animation.hpp"

namespace tr::gp::ecs::AnimationSystem {
   void update(entt::registry& registry) {
      const auto view = registry.view<Animation>();

      for (auto [entity, data] : view.each()) {
         float dt = 1.f / 60.f;
         auto newTime = data.timeRatio + dt * (1.f / data.animation.duration());

         auto samplingJob = ozz::animation::SamplingJob{};
         samplingJob.animation = &data.animation;
         samplingJob.context = &data.context;
         samplingJob.ratio = newTime;
         samplingJob.output = ozz::make_span(data.locals);
         if (!samplingJob.Run()) {
            Log::warn << "Sampling job fail" << std::endl;
         }

         auto ltmJob = ozz::animation::LocalToModelJob{};
         ltmJob.skeleton = &data.skeleton;
         ltmJob.input = ozz::make_span(data.locals);
         ltmJob.output = ozz::make_span(data.models);
         if (!ltmJob.Run()) {
            Log::warn << "Ltm Job Fail" << std::endl;
         }
         data.timeRatio = newTime;
      }
   }
}