#include "ctx/ResourceLoader.hpp"

#include "ctx/RenderObjectData.hpp"
#include "gfx/Handles.hpp"

namespace tr::ctx {

   ResourceLoader::ResourceLoader(gfx::Renderer& renderer) : renderer(renderer) {
   }

   ResourceLoader::~ResourceLoader() {
   }

   std::future<gfx::RenderObjectHandle> ResourceLoader::loadGltf(std::filesystem::path& path) {
      return executor.async([&path]() { return gfx::RenderObjectHandle{3}; });
   }

   std::future<gfx::MeshMaterialHandle> ResourceLoader::uploadRenderObjectData(
       const gfx::RenderObjectHandle handle) {
   }

}