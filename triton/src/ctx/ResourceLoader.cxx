#include "ctx/ResourceLoader.hpp"

#include "ctx/RenderObjectData.hpp"
#include "gfx/Handles.hpp"
#include "gfx/geometry/Vertex.hpp"

#include <fastgltf/core.hpp>
#include <fastgltf/util.hpp>

namespace tr::ctx {

   using Vertex = gfx::Geometry::Vertex;

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
      if (auto error = asset.error(); error != fastgltf::Error::None) {
         std::stringstream ss{};
         ss << "Failed to load gltf file: " << path.string() << fastgltf::to_underlying(error)
            << std::endl;
         throw std::runtime_error(ss.str());
      }

      std::vector<RenderObjectData> renderObjectData;

      for (auto& mesh : asset->meshes) {
         for (auto&& p : mesh.primitives) {
            {
               auto& indexAccessor = asset->accessors[p.indicesAccessor.value()];
               indices.reserve(indices.size() +);
            }
         }
      }

      return gfx::RenderObjectHandle{3};
   }
}
