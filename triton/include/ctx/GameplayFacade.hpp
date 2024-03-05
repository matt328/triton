#pragma once

#include "gp/GameplaySystem.hpp"
#include "gp/ecs/component/Transform.hpp"
#include "gp/ecs/component/Camera.hpp"
#include <entt/entt.hpp>

namespace tr::ctx {
   template <typename T>
   using OptionalRef = std::optional<std::reference_wrapper<T>>;

   struct EditorInfoComponent {
      std::string name;
      std::optional<std::string> sourceMesh;
      std::optional<std::string> sourceTexture;
   };

   class GameplayFacade {
    public:
      GameplayFacade(gp::GameplaySystem& gameplaySystem,
                     gfx::Renderer& renderer,
                     bool debugEnabled = false);
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

      template <typename T>
      OptionalRef<T> getComponent(gp::EntityType entityId) {
         if (gameplaySystem.registry->all_of<T>(entityId)) {
            auto& v = gameplaySystem.registry->get<T>(entityId);
            return OptionalRef<T>{std::ref(v)};
         } else {
            return {};
         }
      }

      OptionalRef<gp::ecs::Transform> getEntityTransform(gp::EntityType entityId);

      EditorInfoComponent& getEditorInfo(gp::EntityType entityId);

      OptionalRef<gp::ecs::Camera> getCameraComponent(const gp::EntityType entity);

      std::string& getActiveCameraName();

    private:
      bool debugEnabled{};
      gp::GameplaySystem& gameplaySystem;
      gfx::Renderer& renderer;
      std::vector<gp::EntityType> allEntities;
   };

}
