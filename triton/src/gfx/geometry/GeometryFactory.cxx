#include "GeometryFactory.hpp"

#include "GeometryData.hpp"
#include "Vertex.hpp"
#include "ct/HeightField.hpp"

namespace tr::gfx::geo {

   GeometryFactory::GeometryFactory()
       : imageGen{imageRandomDevice()},
         imageDistribution(1, 10000),
         geometryGen{geometryRandomDevice()},
         geometryDistribution(1, 10000) {
   }

   GeometryFactory::~GeometryFactory() {
   }

   void GeometryFactory::unload(const TexturedGeometryHandle& handle) {
      for (const auto& p : handle) {
         geometryDataMap.erase(p.first);
         imageDataMap.erase(p.second);
      }
   }

   auto GeometryFactory::createGeometryFromHeightfield(const ct::HeightField& heightField)
       -> TexturedGeometryHandle {
      auto vertices = std::vector<Vertex>{};
      auto indices = std::vector<uint32_t>{};
      auto width = heightField.getWidth();

      const float scaleFactor = 25.f;

      for (int x = 0; x < width; x++) {
         for (int y = 0; y < width; y++) {
            Vertex vert{};
            vert.pos = glm::vec4(static_cast<float>(x) * scaleFactor,
                                 static_cast<float>(heightField.valueAt(x, y) * scaleFactor),
                                 static_cast<float>(y) * scaleFactor,
                                 1.0f);
            vert.normal = glm::zero<glm::vec3>();
            vert.uv = glm::vec2(0.f);
            vert.color = glm::vec4(1.0f);
            vertices.push_back(vert);
         }
      }
      for (int y = 0; y < width - 1; ++y) {
         for (int x = 0; x < width - 1; ++x) {
            auto start = y * width + x;
            indices.push_back(start);
            indices.push_back(start + 1);
            indices.push_back(start + width);
            indices.push_back(start + 1);
            indices.push_back(start + 1 + width);
            indices.push_back(start + width);
         }
      }

      // Calculate face normals and accumulate to vertex normals
      for (size_t i = 0; i < indices.size(); i += 3) {
         const auto& v0 = vertices[indices[i]].pos;
         const auto& v1 = vertices[indices[i + 1]].pos;
         const auto& v2 = vertices[indices[i + 2]].pos;

         auto edge1 = v1 - v0;
         auto edge2 = v2 - v0;
         auto faceNormal = glm::normalize(glm::cross(edge1, edge2));
         for (size_t j = 0; j < 3; ++j) {
            vertices[indices[i + j]].normal += faceNormal;
         }
      }

      // Normalize normals
      for (auto& vertex : vertices) {
         vertex.normal = glm::normalize(vertex.normal);
      }

      const auto imageHandle = imageDistribution(imageGen);
      const auto data = std::vector<unsigned char>(4, 255);
      imageDataMap.emplace(imageHandle, ImageData{data, 1, 1, 4});

      const auto geometryHandle = geometryDistribution(geometryGen);
      geometryDataMap.emplace(geometryHandle, GeometryData{vertices, indices});

      return {{geometryHandle, imageHandle}};
   }

   auto GeometryFactory::generateNormal(int x, int y, const ct::HeightField& heightField)
       -> glm::vec3 {
      int left = std::max(x - 1, 0);
      int right = std::min(x + 1, heightField.getWidth() - 1);
      int up = std::max(y - 1, 0);
      int down = std::min(y + 1, heightField.getWidth() - 1);

      float dx = heightField.valueAt(y, right) - heightField.valueAt(y, left);
      float dy = heightField.valueAt(down, x) - heightField.valueAt(up, x);

      return glm::normalize(glm::vec3(-dx, 2.f, dy));
   }

   /// Creates Vertex, Index and Image data
   auto GeometryFactory::loadGeometryFromGltf(const std::filesystem::path& filename)
       -> TexturedGeometryHandle {
      ZoneNamedN(a, "Load Model Internal", true);
      auto handles = TexturedGeometryHandle{};

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
            auto loadedTextureIndices = std::unordered_map<int, ImageHandle>{};

            auto texturedGeometryHandle = TexturedGeometryHandle{};

            const auto& scene = model.scenes[model.defaultScene];
            for (const auto& nodeIndex : scene.nodes) {
               parseNode(model,
                         model.nodes[nodeIndex],
                         loadedTextureIndices,
                         texturedGeometryHandle);
            }
            return texturedGeometryHandle;
         }
      }
   }

   auto GeometryFactory::parseNode(const tinygltf::Model& model,
                                   const tinygltf::Node& node,
                                   std::unordered_map<int, ImageHandle>& loadedTextureIndices,
                                   TexturedGeometryHandle& handle) -> void {
      if (node.mesh != -1) {
         const auto& mesh = model.meshes[node.mesh];
         for (const auto& primitive : mesh.primitives) {
            const auto geometryHandle = createGeometry(model, primitive);

            auto imageHandle = ImageHandle{};

            const auto materialIndex = primitive.material;
            const auto& material = model.materials[materialIndex];

            const auto& baseColorTextureIndex =
                material.pbrMetallicRoughness.baseColorTexture.index;

            auto it = loadedTextureIndices.find(baseColorTextureIndex);
            if (it != loadedTextureIndices.end()) {
               imageHandle = it->second;
            } else {
               imageHandle = createTexture(model, baseColorTextureIndex);
               loadedTextureIndices.insert({baseColorTextureIndex, imageHandle});
            }

            handle.insert({geometryHandle, imageHandle});
         }
      }
      // Exit Criteria is node.children is empty
      for (auto& child : node.children) {
         auto& node = model.nodes[child];
         parseNode(model, node, loadedTextureIndices, handle);
      }
   }

   auto GeometryFactory::createGeometry(const tinygltf::Model& model,
                                        const tinygltf::Primitive& primitive) -> GeometryHandle {
      ZoneNamedN(a, "Create Geometry", true);
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

      std::vector<geo::Vertex> vertices;
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

         const auto geometryHandle = geometryDistribution(geometryGen);
         geometryDataMap.emplace(geometryHandle, GeometryData{vertices, indices});

         return geometryHandle;
      }
   }

   auto GeometryFactory::createTexture(const tinygltf::Model& model, std::size_t textureIndex)
       -> ImageHandle {
      ZoneNamedN(a, "createTexture", true);
      const auto& texture = model.textures[textureIndex];
      const auto& image = model.images[texture.source];

      const auto imageHandle = imageDistribution(imageGen);
      imageDataMap.emplace(imageHandle,
                           ImageData{image.image, image.width, image.height, image.component});

      return imageHandle;
   }

   [[nodiscard]] auto GeometryFactory::getGeometryData(const GeometryHandle& handle)
       -> GeometryData& {
      return geometryDataMap.at(handle);
   }

   [[nodiscard]] auto GeometryFactory::getImageData(const ImageHandle& handle) -> ImageData& {
      return imageDataMap.at(handle);
   }

}