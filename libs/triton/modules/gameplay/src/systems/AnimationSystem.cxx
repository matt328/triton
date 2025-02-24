#include "AnimationSystem.hpp"

#include "gp/components/Animation.hpp"

namespace tr {

constexpr auto TimeRatio = 0.005f;

AnimationSystem::AnimationSystem(std::shared_ptr<AssetManager> newAssetManager,
                                 std::shared_ptr<EntityService> newEntityService)
    : assetManager{std::move(newAssetManager)}, entityService{std::move(newEntityService)} {
}

auto AnimationSystem::update() const -> void {

  const auto animationFn = [this]([[maybe_unused]] entt::entity entity, Animation& animationData) {
    if (animationData.renderBindPose) {
      for (auto& model : animationData.models) {
        model = ozz::math::Float4x4::identity();
      }
      return;
    }

    if (animationData.playing) {
      animationData.timeRatio += TimeRatio;
      if (animationData.timeRatio >= 1.f) {
        animationData.timeRatio = 0.f;
      }
    }

    const auto& animation = assetManager->getAnimation(animationData.animationHandle);
    const auto& skeleton = assetManager->getSkeleton(animationData.skeletonHandle);

    // This should do this once, then be a no op on subsequent invocations
    animationData.context.Resize(skeleton.num_joints());
    animationData.locals.resize(skeleton.num_soa_joints(), ozz::math::SoaTransform::identity());
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
  };

  entityService->updateAnimations(animationFn);
}

}
