#pragma once

#include "ozz/base/maths/soa_transform.h"
namespace tr::gp::ska {

   struct Animations {
      Animations() {
         // Load Skeleton
         {
            const auto filename =
                std::string{"C:/Users/Matt/Projects/game-assets/animations/skeleton.ozz"};
            Log::debug << "Loading skeleton archive " << filename << "." << std::endl;

            ozz::io::File file(filename.c_str(), "rb");

            if (!file.opened()) {
               Log::error << "Failed to open skeleton file " << filename << "." << std::endl;
            }

            ozz::io::IArchive archive(&file);
            if (!archive.TestTag<ozz::animation::Skeleton>()) {
               Log::error << "Failed to load skeleton instance from file " << filename << "."
                          << std::endl;
            }
            archive >> skeleton;
         }

         { // Load Animation
            const auto filename =
                std::string{"C:/Users/Matt/Projects/game-assets/animations/idleAnimation.ozz"};
            Log::debug << "Loading animation archive: " << filename << "." << std::endl;
            ozz::io::File file(filename.c_str(), "rb");
            if (!file.opened()) {
               Log::error << "Failed to open animation file " << filename << "." << std::endl;
            }
            ozz::io::IArchive archive(&file);
            if (!archive.TestTag<ozz::animation::Animation>()) {
               Log::error << "Failed to load animation instance from file " << filename << "."
                          << std::endl;
            }

            archive >> animation;
         }

         // Sanity Check
         if (skeleton.num_joints() != animation.num_tracks()) {
            Log::warn << "Joints in skeleton have to match tracks in animation" << std::endl;
         }

         // Allocate runtime buffers up front
         const auto numSoaJoints = skeleton.num_soa_joints();
         locals.resize(numSoaJoints, ozz::math::SoaTransform::identity());

         const auto numJoints = skeleton.num_joints();
         models.resize(numJoints, ozz::math::Float4x4::identity());

         context.Resize(numJoints);
      }
      void update(float dt) {
         auto newTime = timeRatio + dt * 1.0f / animation.duration();

         auto samplingJob = ozz::animation::SamplingJob{};
         samplingJob.animation = &animation;
         samplingJob.context = &context;
         samplingJob.ratio = newTime;
         samplingJob.output = ozz::make_span(locals);
         if (!samplingJob.Run()) {
            Log::warn << "Sampling job fail" << std::endl;
         }

         auto ltmJob = ozz::animation::LocalToModelJob{};
         ltmJob.skeleton = &skeleton;
         ltmJob.input = ozz::make_span(locals);
         ltmJob.output = ozz::make_span(models);
         if (!ltmJob.Run()) {
            Log::warn << "Ltm Job Fail" << std::endl;
         }
         timeRatio = newTime;
      }

      float timeRatio{};

      ozz::animation::Skeleton skeleton;
      ozz::animation::Animation animation;
      ozz::animation::SamplingJob::Context context;
      ozz::vector<ozz::math::SoaTransform> locals;
      ozz::vector<ozz::math::Float4x4> models;
   };
}