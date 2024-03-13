#include "ctx/ResourceLoader.hpp"

#include "ctx/RenderObjectData.hpp"
#include "gfx/Handles.hpp"

#include <fastgltf/core.hpp>

namespace tr::ctx {

   ResourceLoader::ResourceLoader(gfx::Renderer& renderer) : renderer(renderer) {
   }

   ResourceLoader::~ResourceLoader() {
   }

   std::future<gfx::RenderObjectHandle> ResourceLoader::loadGltf(std::filesystem::path& path) {
      return executor.async([&path]() { return ResourceLoader::loadGltfInt(path); });
   }

   std::future<gfx::MeshMaterialHandle> ResourceLoader::uploadRenderObjectData(
       const gfx::RenderObjectHandle handle) {
   }

   gfx::RenderObjectHandle ResourceLoader::loadGltfInt(const std::filesystem::path& path) {
      fastgltf::Parser parser;
      fastgltf::GltfDataBuffer data;
      data.loadFromFile(path);

      auto asset = parser.loadGltf(&data, path.parent_path());
      if (auto error = asset.error(); error != fastgltf::Error::None) {}

      return gfx::RenderObjectHandle{3};
   }
}
