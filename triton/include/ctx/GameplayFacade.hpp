#pragma once

#include "gp/GameplaySystem.hpp"

namespace tr::ctx {
   class GameplayFacade {
    public:
      GameplayFacade(gp::GameplaySystem& gameplaySystem, gfx::Renderer& renderer);
      ~GameplayFacade();

      GameplayFacade(const GameplayFacade&) = delete;
      GameplayFacade(GameplayFacade&&) = delete;
      GameplayFacade& operator=(const GameplayFacade&) = delete;
      GameplayFacade& operator=(GameplayFacade&&) = delete;

      gp::EntityType createStaticMeshEntity(std::string meshFile, std::string textureFile);
      gp::EntityType createCamera(uint32_t width,
                                  uint32_t height,
                                  float fov,
                                  float zNear,
                                  float zFar,
                                  glm::vec3 position);
      void setCurrentCamera(gp::EntityType currentCamera);

    private:
      gp::GameplaySystem& gameplaySystem;
      gfx::Renderer& renderer;
   };

}
