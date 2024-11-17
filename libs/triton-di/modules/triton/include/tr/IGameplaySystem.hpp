#pragma once

#include "cm/EntitySystemTypes.hpp"
#include "cm/RenderData.hpp"

namespace tr::gp {

   using RenderDataTransferHandler = std::function<void(cm::gpu::RenderData&)>;

   class IGameplaySystem {
    public:
      explicit IGameplaySystem() = default;
      virtual ~IGameplaySystem() = default;

      IGameplaySystem(const IGameplaySystem&) = default;
      IGameplaySystem(IGameplaySystem&&) = delete;
      auto operator=(const IGameplaySystem&) -> IGameplaySystem& = default;
      auto operator=(IGameplaySystem&&) -> IGameplaySystem& = delete;

      virtual void update() = 0;
      virtual void fixedUpdate() = 0;

      virtual void setRenderDataTransferHandler(const RenderDataTransferHandler& handler) = 0;

      virtual auto createStaticModelEntity(std::string filename) -> cm::EntityType = 0;
      virtual auto createAnimatedModelEntity(std::string modelFilename,
                                             std::string skeletonFilename,
                                             std::string animationFilename) -> cm::EntityType = 0;
      virtual auto createTerrain() -> cm::EntityType = 0;
   };
}