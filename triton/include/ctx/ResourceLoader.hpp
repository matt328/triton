#pragma once

#include "gfx/Handles.hpp"
#include <taskflow/core/taskflow.hpp>

namespace tr::gfx {
   class Renderer;
}

namespace tr::ctx {

   class ModelInfo;

   class ResourceLoader {
    public:
      ResourceLoader(gfx::Renderer& renderer);
      ~ResourceLoader();

      ResourceLoader(const ResourceLoader&) = delete;
      ResourceLoader(ResourceLoader&&) = delete;
      ResourceLoader& operator=(const ResourceLoader&) = delete;
      ResourceLoader& operator=(ResourceLoader&&) = delete;

      /// Loads a GLTF file into memory asynchronously and returns a handle to it in a future
      std::future<gfx::ModelInfoHandle> loadGltf(std::filesystem::path& path);

      std::future<gfx::MeshMaterialHandle> loadGltfAndUpload(std::filesystem::path& path);

      /// Asynchronously uploads the mesh and material's resources to the GPU, returning a future
      /// containing a MeshMaterialHandle when it's ready to be rendered.
      std::future<gfx::MeshMaterialHandle> uploadRenderObjectData(
          const gfx::ModelInfoHandle handle);

    private:
      gfx::Renderer& renderer;
      tf::Executor executor{2};
      tf::Taskflow taskflow;
      std::vector<ModelInfo> modelInfoList;

      gfx::ModelInfoHandle loadGltfInt(const std::filesystem::path& path);
      gfx::MeshMaterialHandle uploadModelInfoInt(const gfx::ModelInfoHandle modelHandle);
   };
}
