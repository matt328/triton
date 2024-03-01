#pragma once

#include "gp/GameplaySystem.hpp"
#include "gp/ecs/Transform.hpp"
#include <optional>

namespace tr::ctx {

   struct NameComponent {
      std::string name;
   };

   class GameplayFacade {
    public:
      GameplayFacade(gp::GameplaySystem& gameplaySystem, gfx::Renderer& renderer);
      ~GameplayFacade();

      GameplayFacade(const GameplayFacade&) = delete;
      GameplayFacade(GameplayFacade&&) = delete;
      GameplayFacade& operator=(const GameplayFacade&) = delete;
      GameplayFacade& operator=(GameplayFacade&&) = delete;

      gp::EntityType createStaticMeshEntity(std::string meshFile,
                                            std::string textureFile,
                                            std::optional<std::string> name = std::nullopt);

      gp::EntityType createCamera(uint32_t width,
                                  uint32_t height,
                                  float fov,
                                  float zNear,
                                  float zFar,
                                  glm::vec3 position,
                                  std::optional<std::string> name = std::nullopt);
      void setCurrentCamera(gp::EntityType currentCamera);

      [[nodiscard]] std::vector<gp::EntityType>& getAllEntities();
      gp::ecs::Transform& getEntityPosition(gp::EntityType entityId);
      NameComponent& getEntityName(gp::EntityType entityId);

    private:
      gp::GameplaySystem& gameplaySystem;
      gfx::Renderer& renderer;
      std::vector<gp::EntityType> allEntities;
   };

}
