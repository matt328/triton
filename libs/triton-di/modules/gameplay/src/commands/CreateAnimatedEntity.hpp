#pragma once
#include <gp/components/Animation.hpp>

namespace tr::gp::cmd {
   class CreateAnimatedEntity final
       : public ICommand<entt::registry&, const std::shared_ptr<gfx::ResourceManager>&> {
    public:
      explicit CreateAnimatedEntity(AnimatedModelData newModelData)
          : animatedModelData{std::move(newModelData)} {
      }

      void execute(entt::registry& registry,
                   const std::shared_ptr<gfx::ResourceManager>& resourceManager) const override {
         auto modelData =
             resourceManager->createModel(std::filesystem::path(animatedModelData.modelFilename));

         // TODO(matt) throw an exception here.
         assert(modelData.skinData.has_value());

         modelData.animationData = std::make_optional(cm::AnimationData{
             .skeletonHandle = resourceManager->loadSkeleton(animatedModelData.skeletonFilename),
             .animationHandle =
                 resourceManager->loadAnimation(animatedModelData.animationFilename)});

         const auto entity = registry.create();
         registry.emplace<cmp::Animation>(entity,
                                          modelData.animationData->animationHandle,
                                          modelData.animationData->skeletonHandle,
                                          modelData.skinData->jointMap,
                                          modelData.skinData->inverseBindMatrices);
         registry.emplace<cmp::Transform>(entity);
         registry.emplace<cmp::Renderable>(entity, std::vector{modelData.meshData});
         registry.emplace<cmp::EditorInfo>(entity,
                                           animatedModelData.entityName.value_or("Unnamed Entity"));
      }

    private:
      AnimatedModelData animatedModelData;
   };
}