#include "ResourceManager.hpp"

#include "gfx/geometry/Mesh.hpp"
#include "gfx/geometry/Vertex.hpp"
#include "gfx/textures/Texture.hpp"
#include "gfx/vma_raii.hpp"
#include "gfx/GraphicsDevice.hpp"
#include <fastgltf/tools.hpp>
#include <glm/detail/qualifier.hpp>

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

   ModelHandle ResourceManager::loadModel(const std::filesystem::path& filename) {
      static constexpr auto supportedExtensions = fastgltf::Extensions::KHR_mesh_quantization;

      fastgltf::Parser parser{supportedExtensions};

      constexpr auto gltfOptions =
          fastgltf::Options::DontRequireValidAssetMember | fastgltf::Options::AllowDouble |
          fastgltf::Options::LoadGLBBuffers | fastgltf::Options::LoadExternalBuffers |
          fastgltf::Options::LoadExternalImages | fastgltf::Options::GenerateMeshIndices;

      fastgltf::GltfDataBuffer data{};
      data.loadFromFile(filename);

      auto asset = parser.loadGltf(&data, filename.parent_path(), gltfOptions);
      if (asset.error() != fastgltf::Error::None) {
         Log::error << "Failed to load glTF: " << fastgltf::getErrorMessage(asset.error())
                    << std::endl;
         throw std::runtime_error("Failed to load glTF");
      }

      const auto& a = asset.get().images[1];

      Log::debug << a.name << std::endl;
      auto materials = std::set<size_t>{};
      auto textures = std::set<size_t>{};
      auto samplers = std::set<size_t>{};

      Log::info << "textures size: " << asset->textures.size() << std::endl;
      Log::info << "materials size: " << asset->materials.size() << std::endl;

      auto loadedTextureIndices = std::unordered_map<std::size_t, TextureHandle>{};

      auto modelHandle = ModelHandle{};

      for (const auto& scene : asset->scenes) {
         for (const auto& nodeIndex : scene.nodeIndices) {

            const auto& node = asset->nodes[nodeIndex];
            const auto& mesh = asset->meshes[node.meshIndex.value()];

            int i = 0;
            for (const auto& primitive : mesh.primitives) {

               const auto meshHandle = createMesh(asset.get(), primitive);
               auto textureHandle = TextureHandle{}; // Init this to a debug texture

               const auto materialIndex = primitive.materialIndex;

               if (materialIndex.has_value()) {
                  const auto& material = asset->materials[materialIndex.value()];
                  const auto& baseColorTexture = material.pbrData.baseColorTexture;

                  if (baseColorTexture.has_value()) {
                     auto it = loadedTextureIndices.find(baseColorTexture.value().textureIndex);
                     if (it != loadedTextureIndices.end()) {
                        textureHandle = it->second;
                     } else {
                        textureHandle = createTexture(asset.get(),
                                                      baseColorTexture.value().textureIndex,
                                                      filename.parent_path());
                        Log::info << "created a texture. it's handle is " << textureHandle
                                  << std::endl;
                        loadedTextureIndices.insert(
                            {baseColorTexture.value().textureIndex, textureHandle});
                     }
                  }
               }
               modelHandle.insert({i, textureHandle});
               i++;
            }
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
         fastgltf::iterateAccessor<glm::vec3>(asset, accessor, [&](glm::vec3 v) {
            auto newVertex = Geometry::Vertex{};
            newVertex.pos = v;
            vertices.push_back(newVertex);
         });

         auto normals = primitive.findAttribute("NORMAL");
         if (normals != primitive.attributes.end()) {
            fastgltf::iterateAccessorWithIndex<glm::vec3>(
                asset,
                asset.accessors[(*normals).second],
                [&](glm::vec3 v, size_t index) { vertices[index].normal = v; });
         }

         auto uv = primitive.findAttribute("TEXCOORD_O");
         if (uv != primitive.attributes.end()) {
            fastgltf::iterateAccessorWithIndex<glm::vec2>(
                asset,
                asset.accessors[(*uv).second],
                [&](glm::vec2 v, size_t index) { vertices[index].uv = v; });
         }
      }

      const auto mesh = std::make_unique<Geometry::Mesh<Geometry::Vertex, uint32_t>>(
          graphicsDevice.getAllocator(),
          vertices,
          indices,
          graphicsDevice.getAsyncTransferContext());

      return static_cast<MeshHandle>(3);
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