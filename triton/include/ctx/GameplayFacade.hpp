#pragma once

#include "gfx/Handles.hpp"
#include "gp/GameplaySystem.hpp"
#include "gp/ecs/component/Transform.hpp"
#include "gp/ecs/component/Camera.hpp"
#include <entt/entt.hpp>

namespace tr::ctx {
   template <typename T>
   using OptionalRef = std::optional<std::reference_wrapper<T>>;

   using MeshHandles = std::unordered_map<gfx::MeshHandle, gfx::TextureHandle>;

   struct EditorInfoComponent {
      std::string name;
      std::optional<std::string> sourceMesh;
      std::optional<std::string> sourceTexture;
   };

   class GameplayFacade {
    public:
      GameplayFacade(gp::GameplaySystem& gameplaySystem,
                     gfx::RenderContext& renderer,
                     bool debugEnabled = false);
      ~GameplayFacade();

      GameplayFacade(const GameplayFacade&) = delete;
      GameplayFacade(GameplayFacade&&) = delete;
      GameplayFacade& operator=(const GameplayFacade&) = delete;
      GameplayFacade& operator=(GameplayFacade&&) = delete;

      auto createSkinnedModelEntity(const gfx::SkinnedModelHandle model) -> gp::EntityType;

      auto createStaticMultiMeshEntity(const MeshHandles meshes) -> gp::EntityType;

      auto createTerrainEntity(const MeshHandles meshes) -> gp::EntityType;

      auto createTerrainMesh(const uint32_t size) -> std::future<gfx::ModelHandle>;

      auto createCamera(uint32_t width,
                        uint32_t height,
                        float fov,
                        float zNear,
                        float zFar,
                        glm::vec3 position,
                        std::optional<std::string> name = std::nullopt) -> gp::EntityType;

      auto setCurrentCamera(gp::EntityType currentCamera) -> void;

      [[nodiscard]] auto getAllEntities() -> std::vector<gp::EntityType>&;

      template <typename T>
      auto getComponent(gp::EntityType entityId) -> OptionalRef<T> {
         if (gameplaySystem.registry->all_of<T>(entityId)) {
            auto& v = gameplaySystem.registry->get<T>(entityId);
            return OptionalRef<T>{std::ref(v)};
         } else {
            return {};
         }
      }

      auto getEntityTransform(gp::EntityType entityId) -> OptionalRef<gp::ecs::Transform>;

      auto getEditorInfo(gp::EntityType entityId) -> EditorInfoComponent&;

      auto getCameraComponent(const gp::EntityType entity) -> OptionalRef<gp::ecs::Camera>;

      auto getActiveCameraName() -> std::string&;

      auto clear() -> void;

      auto loadModelAsync(const std::filesystem::path& path) -> std::future<gfx::ModelHandle>;

      auto loadSkinnedModelAsync(const std::filesystem::path& modelPath,
                                 const std::filesystem::path& skeletonPath,
                                 const std::filesystem::path& animationPath)
          -> std::future<gfx::SkinnedModelHandle>;

    private:
      bool debugEnabled{};
      gp::GameplaySystem& gameplaySystem;
      gfx::RenderContext& renderer;
      std::vector<gp::EntityType> allEntities;
   };

}
