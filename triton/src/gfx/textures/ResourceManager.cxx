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

      taskQueue = std::make_unique<util::TaskQueue>();

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

   std::future<ModelHandle> ResourceManager::loadModelAsync(const std::filesystem::path& filename) {
      return taskQueue->enqueue([this, filename]() { return loadModelInt(filename); });
   }

   ModelHandle ResourceManager::loadModelInt(const std::filesystem::path& filename) {
      ZoneNamedN(a, "Load Model Internal", true);

      auto modelHandles = ModelHandle{};

      {
         ZoneNamedN(b, "Loading glTF File", true);

         using namespace tinygltf;

         Model model;
         TinyGLTF loader;
         std::string err;
         std::string warn;

         {
            ZoneNamedN(a, "Reading ASCII File", true);
            bool ret = loader.LoadASCIIFromFile(&model, &err, &warn, filename.string());
            if (!warn.empty()) {
               throw std::runtime_error(warn);
            }

            if (!err.empty()) {
               throw std::runtime_error(err);
            }

            if (!ret) {
               Log::error << "Failed to parse glTF file" << std::endl;
               throw std::runtime_error("Failed to parse glTF file");
            }
         }
         {
            ZoneNamedN(b, "Parsing glTF", true);
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

                  auto it = loadedTextureIndices.find(baseColorTextureIndex);
                  if (it != loadedTextureIndices.end()) {
                     textureHandle = it->second;
                  } else {
                     textureHandle = createTexture(model, baseColorTextureIndex);
                     loadedTextureIndices.insert({baseColorTextureIndex, textureHandle});
                  }

                  modelHandle.insert({meshHandle, textureHandle});
               }
            }
            return modelHandle;
         }
      }

      return modelHandles;
   }

   /// Brute force this for now, create a single Mesh per Primitive in the file
   MeshHandle ResourceManager::createMesh(const tinygltf::Model& model,
                                          const tinygltf::Primitive& primitive) {
      ZoneNamedN(a, "createMesh", true);
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
            const auto& vertexCount = accessor.count;
            const auto& view = model.bufferViews[accessor.bufferView];
            const auto dataOffset = accessor.byteOffset + view.byteOffset;
            const auto& buffer = model.buffers[view.buffer];

            // NOLINTNEXTLINE
            const auto& data = reinterpret_cast<const float*>(&buffer.data[dataOffset]);

            vertices.resize(vertexCount);

            if (attribute.first.compare("POSITION") == 0) {
               for (size_t i = 0; i < vertexCount; i++) {
                  vertices[i].pos = glm::vec4(glm::make_vec3(&data[i * 3]), 1.f);
               }
            }
            if (attribute.first.compare("NORMAL") == 0) {
               for (size_t i = 0; i < vertexCount; i++) {
                  vertices[i].normal = glm::make_vec3(&data[i * 3]);
               }
            }
            if (attribute.first.compare("TEXCOORD_0") == 0) {
               for (size_t i = 0; i < vertexCount; i++) {
                  vertices[i].uv = glm::make_vec2(&data[i * 2]);
               }
            }
         }

         const auto position = meshList.size();

         meshList.emplace_back(std::make_unique<Geometry::Mesh<Geometry::Vertex, uint32_t>>(
             graphicsDevice.getAllocator(),
             vertices,
             indices,
             graphicsDevice.getAsyncTransferContext()));

         return static_cast<MeshHandle>(position);
      }
   }

   void ResourceManager::accessTextures(
       std::function<void(const std::vector<vk::DescriptorImageInfo>&)> fn) const {
      if (textureInfoList.empty()) {
         return;
      }
      std::lock_guard<LockableBase(std::mutex)> lock(textureListMutex);
      LockableName(textureListMutex, "Access", 6);
      LockMark(textureListMutex);
      fn(textureInfoList);
   }

   TextureHandle ResourceManager::createTexture(const tinygltf::Model& model,
                                                std::size_t textureIndex) {
      ZoneNamedN(a, "createTexture", true);
      const auto& texture = model.textures[textureIndex];
      const auto& image = model.images[texture.source];

      const auto pos = textureList.size();
      {
         ZoneNamedN(b, "UploadTexture", true);
         textureList.emplace_back(
             std::make_unique<Textures::Texture>((void*)image.image.data(),
                                                 image.width,
                                                 image.height,
                                                 image.component,
                                                 graphicsDevice.getAllocator(),
                                                 graphicsDevice.getVulkanDevice(),
                                                 graphicsDevice.getAsyncTransferContext()));
      }
      { // Only need to guard access to the textureInfoList
         ZoneNamedN(c, "Update TextureInfoList", true);
         std::lock_guard<LockableBase(std::mutex)> lock(textureListMutex);
         LockMark(textureListMutex);
         LockableName(textureListMutex, "Mutate", 6);
         textureInfoList.emplace_back(textureList[pos]->getImageInfo());
         return static_cast<TextureHandle>(pos);
      }
   }
}