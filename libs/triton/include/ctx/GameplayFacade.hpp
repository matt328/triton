#pragma once

#include "gfx/Handles.hpp"
#include "gp/GameplaySystem.hpp"
#include "gp/ecs/component/Transform.hpp"
#include "gp/ecs/component/Camera.hpp"
#include "gp/EntitySystemTypes.hpp"

namespace tr::ctx {
   template <typename T>
   using OptionalRef = std::optional<std::reference_wrapper<T>>;

   struct EditorInfoComponent {
      std::string name;
      std::optional<std::string> sourceMesh;
      std::optional<std::string> sourceTexture;
   };

   /*
      This class is a facade over the gameplay api, and the ResourceManager.
      It ensures that as game world objects are created, required gfx resources are created and
      linked to.

      This class should also live in the intermediate module between gp and gfx.
   */
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

      // Entity Creation Methods
      auto createSkinnedModelEntity(const gfx::LoadedSkinnedModelData model) -> gp::EntityType;

      auto createStaticMultiMeshEntity(const gfx::MeshHandles meshes) -> gp::EntityType;

      auto createTerrainEntity(const gfx::MeshHandles meshes) -> gp::EntityType;

      auto createTerrainMesh(const uint32_t size) -> std::future<gfx::ModelHandle>;

      auto createCamera(uint32_t width,
                        uint32_t height,
                        float fov,
                        float zNear,
                        float zFar,
                        glm::vec3 position,
                        std::optional<std::string> name = std::nullopt) -> gp::EntityType;

      auto setCurrentCamera(gp::EntityType currentCamera) -> void;

      auto getAnimationTimeRange(const gfx::AnimationHandle handle) -> std::tuple<float, float>;

      auto clear() -> void;

      // Resource Creation Methods
      /*
         TODO:
         This facade shouldn't expose the two parts of gfx loading and then gp creation. It should
         just be able to tell the gp system to create something, then the gp system should be able
         to tell the gfx system what assets it should use to render the entity.

         In a game application, these assets might be loaded differently, from a pack or something,
         so we'll need to think about what API the editor needs vs what API the game needs, and how
         to factor out a common API that both of those can talk to.
      */
      auto loadModelAsync(const std::filesystem::path& path) -> std::future<gfx::ModelHandle>;

      auto loadSkinnedModelAsync(const std::filesystem::path& modelPath,
                                 const std::filesystem::path& skeletonPath,
                                 const std::filesystem::path& animationPath)
          -> std::future<gfx::LoadedSkinnedModelData>;

      void loadModelResources(const std::filesystem::path& modelPath,
                              const std::filesystem::path& skeletonPath,
                              const std::filesystem::path& animationPath,
                              const std::function<void()> done);

      void update();

    private:
      bool debugEnabled{};
      gp::GameplaySystem& gameplaySystem;
      gfx::RenderContext& renderer;
      std::vector<gp::EntityType> allEntities;

      std::vector<std::future<tr::gfx::ModelHandle>> terrainFutures{};
   };

}
