#pragma once
#include "CommandQueue.hpp"
#include "gp/AssetManager.hpp"
#include "gp/components/Animation.hpp"
#include "gp/components/Transform.hpp"
#include "gp/components/EditorInfo.hpp"
#include "gp/components/Renderable.hpp"
#include "tr/AnimatedModelData.hpp"

namespace tr {
class CreateAnimatedEntity final
    : public ICommand<entt::registry&, const std::shared_ptr<AssetManager>&> {
public:
  explicit CreateAnimatedEntity(AnimatedModelData newModelData)
      : animatedModelData{std::move(newModelData)} {
  }

  void execute(entt::registry& registry,
               const std::shared_ptr<AssetManager>& assetManager) const override {
    auto modelData = assetManager->loadModel(animatedModelData.modelFilename);

    assert(modelData.skinData.has_value());

    modelData.animationData = std::make_optional(AnimationData{
        .skeletonHandle = assetManager->loadSkeleton(animatedModelData.skeletonFilename),
        .animationHandle = assetManager->loadAnimation(animatedModelData.animationFilename)});

    const auto entity = registry.create();
    registry.emplace<Animation>(entity,
                                modelData.animationData->animationHandle,
                                modelData.animationData->skeletonHandle,
                                modelData.skinData->jointMap,
                                modelData.skinData->inverseBindMatrices);
    registry.emplace<Transform>(entity);
    registry.emplace<Renderable>(entity, std::vector{modelData.meshData});
    registry.emplace<EditorInfo>(entity, animatedModelData.entityName.value_or("Unnamed Entity"));
  }

private:
  AnimatedModelData animatedModelData;
};
}
