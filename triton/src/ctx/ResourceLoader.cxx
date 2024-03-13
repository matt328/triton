#include "ctx/ResourceLoader.hpp"

#include "ctx/RenderObjectData.hpp"
#include "gfx/Handles.hpp"
#include "gfx/geometry/Vertex.hpp"

#include <fastgltf/core.hpp>
#include <fastgltf/util.hpp>
#include <fastgltf/tools.hpp>

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
            auto primitive = Primitive{};
            // Parse Indices
            {
               auto& indexAccessor = asset->accessors[p.indicesAccessor.value()];
               primitive.indices.reserve(indexAccessor.count);
               fastgltf::copyFromAccessor<std::uint32_t>(asset.get(),
                                                         indexAccessor,
                                                         primitive.indices.data());
            }
            // Parse Vertex Positions
            {
               auto& posAccessor = asset->accessors[p.findAttribute("POSITION")->second];
               fastgltf::iterateAccessor<glm::vec3>(asset.get(), posAccessor, [&](glm::vec3 v) {
                  Vertex vertex{};
                  vertex.pos = v;
                  primitive.vertices.push_back(vertex);
               });
            }

            // Normals
            auto normals = p.findAttribute("NORMAL");
            if (normals != p.attributes.end()) {
               fastgltf::iterateAccessorWithIndex<glm::vec3>(
                   asset.get(),
                   asset->accessors[(*normals).second],
                   [&](glm::vec3 v, size_t index) { primitive.vertices[index].normal = v; });
            }

            // load UVs
            auto uv = p.findAttribute("TEXCOORD_0");
            if (uv != p.attributes.end()) {
               fastgltf::iterateAccessorWithIndex<glm::vec2>(
                   asset.get(),
                   asset->accessors[(*uv).second],
                   [&](glm::vec2 v, size_t index) { primitive.vertices[index].uv = v; });
            }

            // load vertex colors
            auto colors = p.findAttribute("COLOR_0");
            if (colors != p.attributes.end()) {
               fastgltf::iterateAccessorWithIndex<glm::vec4>(
                   asset.get(),
                   asset->accessors[(*colors).second],
                   [&](glm::vec4 v, size_t index) { primitive.vertices[index].color = v; });
            }
         }
      }

      return gfx::RenderObjectHandle{3};
   }
}
