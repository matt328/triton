#pragma once

#include "gfx/Handles.hpp"

namespace tr::gfx {
   class Renderer;
}

namespace tr::ctx {
   struct RenderObjectData;

   class ResourceLoader {
    public:
      ResourceLoader(gfx::Renderer& renderer);
      ~ResourceLoader();

      ResourceLoader(const ResourceLoader&) = delete;
      ResourceLoader(ResourceLoader&&) = delete;
      ResourceLoader& operator=(const ResourceLoader&) = delete;
      ResourceLoader& operator=(ResourceLoader&&) = delete;

      /// Loads a GLTF file into memory asynchronously and returns a handle to it in a future
      std::future<gfx::RenderObjectHandle> loadGltf(std::filesystem::path& path);

      /// Asynchronously uploads the mesh and material's resources to the GPU, returning a future
      /// containing a MeshMaterialHandle when it's ready to be rendered.
      std::future<gfx::MeshMaterialHandle> uploadRenderObjectData(
          const gfx::RenderObjectHandle handle);

    private:
      gfx::Renderer& renderer;
      tf::Executor executor{2};
      std::vector<RenderObjectData> renderObjectData;

      gfx::RenderObjectHandle loadGltf(const std::filesystem::path& path) {
         return 3;
      }
   };
}
