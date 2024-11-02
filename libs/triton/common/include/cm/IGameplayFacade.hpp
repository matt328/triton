#pragma once

#include "EntitySystemTypes.hpp"

namespace tr::cm {

   struct AnimatedModelInfo {
      std::filesystem::path modelPath;
      std::filesystem::path skeletonPath;
      std::filesystem::path animationPath;
   };

   class IGameplaySystem;
   class IRenderContext;

   class IGameplayFacade {

    public:
      IGameplayFacade(IGameplaySystem& gameplaySystem, IRenderContext& renderer);
      virtual ~IGameplayFacade() = default;

      IGameplayFacade(const IGameplayFacade&) = default;
      IGameplayFacade(IGameplayFacade&&) = delete;
      auto operator=(const IGameplayFacade&) -> IGameplayFacade& = default;
      auto operator=(IGameplayFacade&&) -> IGameplayFacade& = delete;

      virtual void createTerrain() const = 0;

      [[nodiscard]] virtual auto createCamera(uint32_t width,
                                              uint32_t height,
                                              float fov,
                                              float zNear,
                                              float zFar,
                                              const glm::vec3& position,
                                              const std::optional<std::string>& name) const
          -> cm::EntityType = 0;

      virtual auto setCurrentCamera(cm::EntityType currentCamera) const -> void = 0;

      [[nodiscard]] virtual auto createDebugAABB(const glm::vec3& min, const glm::vec3& max) const
          -> cm::EntityType = 0;

      [[nodiscard]] virtual auto createStaticModelEntity(
          const std::filesystem::path& modelPath) const noexcept -> cm::EntityType = 0;

      [[nodiscard]] virtual auto createAnimatedModelEntity(const AnimatedModelInfo& info) const
          -> cm::EntityType = 0;

      virtual auto clear() const -> void = 0;

      virtual void addTerrainCreatedListener(const cm::TerrainCreatedFn& func) = 0;
   };

}
