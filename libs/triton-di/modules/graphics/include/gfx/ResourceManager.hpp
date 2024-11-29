#pragma once

#include "cm/Handles.hpp"

#include <gp/AnimationFactory.hpp>

namespace tr::gfx {
   class IGraphicsDevice;
}

namespace tr::gfx::geo {
   class GeometryData;
   class GeometryFactory;
}

namespace tr::gfx {

   constexpr uint32_t MaxImageSize = 1024 * 1024 * 8;

   class ResourceManager {
    public:
      explicit ResourceManager(std::shared_ptr<IGraphicsDevice> newGraphicsDevice,
                               std::shared_ptr<gp::AnimationFactory> newAnimationFactory);
      ~ResourceManager() = default;

      ResourceManager(const ResourceManager&) = delete;
      ResourceManager(ResourceManager&&) = delete;
      auto operator=(const ResourceManager&) -> ResourceManager& = delete;
      auto operator=(ResourceManager&&) -> ResourceManager& = delete;

      [[nodiscard]] auto createTerrain() const -> std::vector<cm::ModelData>;

      [[nodiscard]] auto createModel(const std::filesystem::path& filename) const noexcept
          -> cm::ModelData;

      [[nodiscard]] auto createAABB(const glm::vec3& min, const glm::vec3& max) const noexcept
          -> cm::ModelData;

      auto createStaticMesh(const geo::GeometryData& geometry) -> cm::MeshHandle;

      [[nodiscard]] auto loadSkeleton(const std::filesystem::path& filename) const noexcept
          -> cm::SkeletonHandle;

      [[nodiscard]] auto loadAnimation(const std::filesystem::path& filename) const noexcept
          -> cm::AnimationHandle;

    private:
      std::shared_ptr<IGraphicsDevice> graphicsDevice;
      std::shared_ptr<gp::AnimationFactory> animationFactory;

      std::unique_ptr<geo::GeometryFactory> geometryFactory;
   };
}
