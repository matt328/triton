#include "ResourceManager.hpp"

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
// #define TINYGLTF_NOEXCEPTION // optional. disable exception handling.
#include "tiny_gltf.h"

#include "gfx/geometry/Mesh.hpp"
#include "gfx/geometry/Vertex.hpp"
#include "gfx/textures/Texture.hpp"
#include "gfx/vma_raii.hpp"
#include "gfx/GraphicsDevice.hpp"

namespace tr::gfx::tx {
   ResourceManager::ResourceManager(const GraphicsDevice& graphicsDevice)
       : graphicsDevice{graphicsDevice} {

      const auto bci = vk::BufferCreateInfo{.size = MaxImageSize,
                                            .usage = vk::BufferUsageFlagBits::eTransferSrc,
                                            .sharingMode = vk::SharingMode::eExclusive};
      const auto aci =
          vma::AllocationCreateInfo{.usage = vma::MemoryUsage::eCpuOnly,
                                    .requiredFlags = vk::MemoryPropertyFlagBits::eHostVisible |
                                                     vk::MemoryPropertyFlagBits::eHostCoherent};

      stagingBuffer =
          graphicsDevice.getAllocator().createBuffer(&bci, &aci, "ResourceManager Staging Buffer");
   }

   ResourceManager::~ResourceManager() {
   }

   // TODO: go back to tinygltf. fastgltf does weird crap corrupting memory profiling
   // and I can't have that.
   ModelHandle ResourceManager::loadModel(const std::filesystem::path& filename) {

      using namespace tinygltf;

      Model model;
      TinyGLTF loader;
      std::string err;
      std::string warn;

      bool ret = loader.LoadASCIIFromFile(&model, &err, &warn, filename.string());
      if (!warn.empty()) {
         // TODO: non-critical exceptions
         throw std::runtime_error(warn);
      }

      if (!err.empty()) {
         throw std::runtime_error(err);
      }

      if (!ret) {
         Log::error << "Failed to parse glTF file" << std::endl;
         throw std::runtime_error("Failed to parse glTF file");
      }

      auto loadedTextureIndices = std::unordered_map<int, TextureHandle>{};

      auto modelHandle = ModelHandle{};

      const auto& scene = model.scenes[model.defaultScene];
      for (const auto& nodeIndex : scene.nodes) {
         const auto& node = model.nodes[nodeIndex];
         const auto& mesh = model.meshes[node.mesh];
         for (const auto& primitive : mesh.primitives) {
            const auto meshHandle = createMesh(model, primitive);

            auto textureHandle = TextureHandle{}; // TODO: init this to a debug texture handle

            const auto materialIndex = primitive.material;
            const auto& material = model.materials[materialIndex];

            const auto& baseColorTextureIndex =
                material.pbrMetallicRoughness.baseColorTexture.index;
            const auto& baseColorTexture = model.textures[baseColorTextureIndex];

            auto it = loadedTextureIndices.find(baseColorTextureIndex);
            if (it != loadedTextureIndices.end()) {
               textureHandle = it->second;
            } else {
               textureHandle = createTexture(model, baseColorTextureIndex, filename.parent_path());
               loadedTextureIndices.insert({baseColorTextureIndex, textureHandle});
            }

            Log::info << "inserting mesh " << meshHandle << " and texture " << textureHandle
                      << std::endl;
            modelHandle.insert({meshHandle, textureHandle});
         }
      }
      return modelHandle;
   }

   /// Brute force this for now, create a single Mesh per Primitive in the file
   MeshHandle ResourceManager::createMesh(const fastgltf::Asset& asset,
                                          const fastgltf::Primitive& primitive) {
      // Load Indices
      std::vector<uint32_t> indices;
      {
         const auto& indexAccessor = asset.accessors[primitive.indicesAccessor.value()];
         indices.resize(indexAccessor.count);
         fastgltf::copyFromAccessor<std::uint32_t>(asset, indexAccessor, indices.data());
      }

      std::vector<Geometry::Vertex> vertices;
      {
         const auto& accessor = asset.accessors[primitive.findAttribute("POSITION")->second];
         vertices.resize(accessor.count);
         fastgltf::iterateAccessorWithIndex<glm::vec3>(asset,
                                                       accessor,
                                                       [&](glm::vec3 v, size_t index) {
                                                          auto newVertex = Geometry::Vertex{};
                                                          newVertex.pos = v;
                                                          vertices[index] = newVertex;
                                                       });

         auto normals = primitive.findAttribute("NORMAL");
         if (normals != primitive.attributes.end()) {
            fastgltf::iterateAccessorWithIndex<glm::vec3>(
                asset,
                asset.accessors[(*normals).second],
                [&](glm::vec3 v, size_t index) { vertices[index].normal = v; });
         }

         auto uv = primitive.findAttribute("TEXCOORD_0");
         if (uv != primitive.attributes.end()) {
            fastgltf::iterateAccessorWithIndex<glm::vec2>(
                asset,
                asset.accessors[(*uv).second],
                [&](glm::vec2 v, size_t index) { vertices[index].uv = v; });
         }
      }
      const auto pos = meshList.size();
      meshList.push_back(std::make_unique<Geometry::Mesh<Geometry::Vertex, uint32_t>>(
          graphicsDevice.getAllocator(),
          vertices,
          indices,
          graphicsDevice.getAsyncTransferContext()));

      return static_cast<MeshHandle>(pos);
   }

   TextureHandle ResourceManager::createTexture(const fastgltf::Asset& asset,
                                                std::size_t textureIndex,
                                                const std::filesystem::path& folder) {
      const auto texture = asset.textures[textureIndex];

      if (!texture.imageIndex.has_value()) {
         Log::info << "unsupported image type found for texture" << std::endl;
         // Return default texture handle here
         return 0;
      }

      const auto& image = asset.images[texture.imageIndex.value()];

      unsigned char* data = nullptr;
      int width{}, height{}, numChannels{};

      const auto desiredChannels = STBI_rgb_alpha;

      std::visit(fastgltf::visitor{
                     []([[maybe_unused]] auto& arg) {},
                     [&](fastgltf::sources::URI& filePath) {
                        if (filePath.fileByteOffset != 0) {
                           throw std::runtime_error(
                               "Offset found in image. We don't do that here.");
                        }
                        if (!filePath.uri.isLocalPath()) {
                           throw std::runtime_error("Nonlocal file. Einstien was wrong.");
                        }
                        const auto imagePath = folder / std::filesystem::path{filePath.uri.path()};
                        data = stbi_load(imagePath.string().c_str(),
                                         &width,
                                         &height,
                                         &numChannels,
                                         STBI_rgb_alpha);
                     },
                     [&](const fastgltf::sources::Array& vector) {
                        data = stbi_load_from_memory(vector.bytes.data(),
                                                     static_cast<int>(vector.bytes.size()),
                                                     &width,
                                                     &height,
                                                     &numChannels,
                                                     STBI_rgb_alpha);
                     },
                     [&](fastgltf::sources::BufferView view) {
                        auto& bufferView = asset.bufferViews[view.bufferViewIndex];
                        auto& buffer = asset.buffers[bufferView.bufferIndex];
                        std::visit(fastgltf::visitor{[]([[maybe_unused]] auto& arg) {},
                                                     [&](fastgltf::sources::Vector vector) {
                                                        data = stbi_load_from_memory(
                                                            vector.bytes.data(),
                                                            static_cast<int>(vector.bytes.size()),
                                                            &width,
                                                            &height,
                                                            &numChannels,
                                                            STBI_rgb_alpha);
                                                     }},
                                   buffer.data);
                     }},
                 image.data);
      if (data == nullptr) {
         Log::warn << "Couldn't find a texture in glft file" << std::endl;
         // Skip loading and just return the handle of the default texture
         data = new unsigned char[1];
         data[0] = 255;
         width = 1;
         height = 1;
         numChannels = 4;
      }

      const auto pos = textureList.size();

      textureList.emplace_back(
          std::make_unique<Textures::Texture>(data,
                                              width,
                                              height,
                                              desiredChannels,
                                              graphicsDevice.getAllocator(),
                                              graphicsDevice.getVulkanDevice(),
                                              graphicsDevice.getAsyncTransferContext()));

      return static_cast<TextureHandle>(pos);
   }
}