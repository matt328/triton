#pragma once

#include "cm/Handles.hpp"

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
      explicit ResourceManager(std::shared_ptr<IGraphicsDevice> newGraphicsDevice);
      ~ResourceManager() = default;

      ResourceManager(const ResourceManager&) = delete;
      ResourceManager(ResourceManager&&) = delete;
      auto operator=(const ResourceManager&) -> ResourceManager& = delete;
      auto operator=(ResourceManager&&) -> ResourceManager& = delete;

      auto createTerrain() const -> std::vector<cm::ModelData>;

      auto createModel(const std::filesystem::path& filename) const noexcept -> cm::ModelData;

      auto createAABB(const glm::vec3& min, const glm::vec3& max) const noexcept -> cm::ModelData;

      auto createStaticMesh(const geo::GeometryData& geometry) -> cm::MeshHandle;

    private:
      std::shared_ptr<IGraphicsDevice> graphicsDevice;
      std::unique_ptr<geo::GeometryFactory> geometryFactory;
   };
}
