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
               textureHandle =
                   3; // createTexture(model, baseColorTextureIndex, filename.parent_path());
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
   MeshHandle ResourceManager::createMesh(const tinygltf::Model& model,
                                          const tinygltf::Primitive& primitive) {
      // Load Indices
      std::vector<uint32_t> indices;
      {
         const auto& accessor = model.accessors[primitive.indices];
         const auto& bufferView = model.bufferViews[accessor.bufferView];
         const auto& buffer = model.buffers[bufferView.buffer];

         // glTF supports different component types of indices
         switch (accessor.componentType) {
            case TINYGLTF_PARAMETER_TYPE_UNSIGNED_INT: {
               auto buf = reinterpret_cast<const uint32_t*>(
                   &buffer.data[accessor.byteOffset + bufferView.byteOffset]);
               for (size_t index = 0; index < accessor.count; index++) {
                  indices.push_back(buf[index]);
               }
               break;
            }
            case TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT: {
               auto buf = reinterpret_cast<const uint16_t*>(
                   &buffer.data[accessor.byteOffset + bufferView.byteOffset]);
               for (size_t index = 0; index < accessor.count; index++) {
                  indices.push_back(buf[index]);
               }
               break;
            }
            case TINYGLTF_PARAMETER_TYPE_UNSIGNED_BYTE: {
               auto buf = reinterpret_cast<const uint8_t*>(
                   &buffer.data[accessor.byteOffset + bufferView.byteOffset]);
               for (size_t index = 0; index < accessor.count; index++) {
                  indices.push_back(buf[index]);
               }
               break;
            }
            default:
               std::stringstream err;
               err << "Index component type " << accessor.componentType;
               throw std::runtime_error(err.str());
         }
      }

      std::vector<Geometry::Vertex> vertices;
      {
         for (const auto& attribute : primitive.attributes) {
            const auto& accessor = model.accessors[attribute.second];
            const auto& view = model.bufferViews[accessor.bufferView];
            if (attribute.first.compare("POSITION") == 0) {
               // NOLINTNEXTLINE
               const auto& v = reinterpret_cast<const float*>(
                   &model.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]);
               const auto& vertexCount = accessor.count;
               for (size_t i = 0; i < vertexCount; i++) {
                  Geometry::Vertex vert{};
                  vert.pos = glm::vec4(glm::make_vec3(&v[i * 3]), 1.f);
                  vertices.push_back(vert);
               }
            }
         }

         return static_cast<MeshHandle>(3);
      }
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