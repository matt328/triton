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
  CreateAnimatedEntity(AnimatedModelData newModelData, std::optional<Transform> newInitialTransform)
      : animatedModelData{std::move(newModelData)}, initialTransform{newInitialTransform} {
  }

  void execute(entt::registry& registry,
               const std::shared_ptr<AssetManager>& assetManager) const override {
    auto loadedModelData = assetManager->loadModel(animatedModelData.modelFilename);

    assert(loadedModelData.skinData.has_value());

    loadedModelData.animationData = std::make_optional(AnimationData{
        .skeletonHandle = assetManager->loadSkeleton(animatedModelData.skeletonFilename),
        .animationHandle = assetManager->loadAnimation(animatedModelData.animationFilename)});

    auto transform = Transform{.rotation = glm::zero<glm::vec3>(),
                               .position = glm::zero<glm::vec3>(),
                               .transformation = glm::identity<glm::mat4>()};

    if (initialTransform.has_value()) {
      transform.position = initialTransform->position;
      transform.rotation = initialTransform->rotation;
    }

    const auto entity = registry.create();
    registry.emplace<Animation>(entity,
                                loadedModelData.animationData->animationHandle,
                                loadedModelData.animationData->skeletonHandle,
                                loadedModelData.skinData->jointMap,
                                loadedModelData.skinData->inverseBindMatrices);
    registry.emplace<Transform>(entity, transform);
    registry.emplace<Renderable>(entity, std::vector{loadedModelData.meshData});
    registry.emplace<EditorInfo>(entity, animatedModelData.entityName.value_or("Unnamed Entity"));
  }

private:
  AnimatedModelData animatedModelData;
  std::optional<Transform> initialTransform;
};
}
