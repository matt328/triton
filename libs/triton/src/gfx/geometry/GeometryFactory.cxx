#include "GeometryFactory.hpp"

#include "GeometryData.hpp"
#include "Vertex.hpp"
#include "ct/HeightField.hpp"
#include "gfx/Handles.hpp"
#include "gfx/geometry/AnimationFactory.hpp"
#include "gfx/geometry/GeometryHandles.hpp"
#include "gfx/geometry/OzzMesh.hpp"
#include <ozz/animation/runtime/skeleton_utils.h>

namespace tr::gfx::geo {

   GeometryFactory::GeometryFactory(AnimationFactory& animationFactory)
       : animationFactory{animationFactory} {
      const auto imageHandle = 0;
      const auto data = std::vector<unsigned char>(4, 255);
      imageDataMap.emplace(imageHandle, ImageData{data, 1, 1, 4});
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

      const auto imageHandle = imageKey.getKey();
      const auto data = std::vector<unsigned char>(4, 255);
      imageDataMap.emplace(imageHandle, ImageData{data, 1, 1, 4});

      const auto geometryHandle = geometryKey.getKey();
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

   auto GeometryFactory::loadSkinnedModel(const std::filesystem::path& modelPath,
                                          const std::filesystem::path& skeletonPath,
                                          const std::filesystem::path& animationPath)
       -> SkinnedGeometryData {
      auto sgd = SkinnedGeometryData{};
      try {

         sgd.skeletonHandle = animationFactory.loadSkeleton(skeletonPath);
         sgd.animationHandle = animationFactory.loadAnimation(animationPath);

         const auto& skeleton = animationFactory.getSkeleton(sgd.skeletonHandle);

         ozz::animation::IterateJointsDF(skeleton, [](int current, int parent) {
            Log::debug << "skeleton order: " << current << std::endl;
         });

         const auto modelHandle = loadGeometryFromGltf(modelPath);

         assert(modelHandle.size() == 1); // Currently only support files with a single mesh

         const auto meshHandle = modelHandle.begin()->first;
         const auto imageHandle = modelHandle.begin()->second;

         sgd.geometryHandle = meshHandle;
         sgd.imageHandle = imageHandle;

         return sgd;
      } catch (const std::exception& ex) {
         Log::error << "Error during loadGeometryFromGltf: " << ex.what() << std::endl;
      }
      return sgd;
   }

   auto GeometryFactory::loadOzzMesh(const std::filesystem::path& filename)
       -> TexturedGeometryHandle {
      ozz::io::File file(filename.string().c_str(), "rb");

      if (!file.opened()) {
         std::stringstream ss{};
         ss << "Failed to open mesh file " << filename.string() << std::endl;
         throw std::runtime_error(ss.str());
      }

      auto meshes = std::vector<ozz::sample::Mesh>{};
      ozz::io::IArchive archive(&file);

      while (archive.TestTag<ozz::sample::Mesh>()) {
         meshes.resize(meshes.size() + 1);
         archive >> meshes.back();
      }

      Log::debug << "loaded " << meshes.size() << "meshes from " << filename << std::endl;

      return {};
   }

   /// Creates Vertex, Index and Image data
   auto GeometryFactory::loadGeometryFromGltf(const std::filesystem::path& filename)
       -> TexturedGeometryHandle {
      ZoneNamedN(a, "Load Model Internal", true);
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

            const auto parentMap = buildParentMap(model);

            const auto& scene = model.scenes[model.defaultScene];
            auto nodeList = std::vector<GltfNode>{};
            for (const auto& nodeIndex : scene.nodes) {
               parseNode(model,
                         model.nodes[nodeIndex],
                         loadedTextureIndices,
                         texturedGeometryHandle,
                         nodeList,
                         nodeIndex,
                         parentMap);
            }

            std::sort(nodeList.begin(), nodeList.end(), [](const GltfNode& a, const GltfNode& b) {
               return a.number < b.number;
            });

            const auto& skin = model.skins[0];
            auto accessorIndex = skin.inverseBindMatrices;

            if (accessorIndex < 0 || accessorIndex >= model.accessors.size()) {
               throw std::runtime_error("Invalid accessor index for inverse bind matrices");
            }

            const auto& accessor = model.accessors[accessorIndex];
            const auto& bufferView = model.bufferViews[accessor.bufferView];
            const auto& buffer = model.buffers[bufferView.buffer];

            if (accessor.type != TINYGLTF_TYPE_MAT4 || accessor.count == 0) {
               throw std::runtime_error("Invalid inverse bind matrices accessor");
            }

            const auto data = reinterpret_cast<const float*>(
                buffer.data.data() + bufferView.byteOffset + accessor.byteOffset);

            auto inverseBindMatrices = std::vector<glm::mat4>{};
            inverseBindMatrices.reserve(accessor.count);

            for (size_t i = 0; i < accessor.count; ++i) {
               glm::mat4 mat = glm::make_mat4(data + i * 16);
               inverseBindMatrices.push_back(mat);
            }

            return texturedGeometryHandle;
         }
      }
   }

   /*
      I cannot figure out why this model is rotated -180 degrees about the z-axis.
      Is it some bs between vulkan and glm being opposite coordinate systems?
   */

   auto GeometryFactory::parseNodeTransform(const tinygltf::Node& node) -> glm::mat4 {
      // If glft file has a matrix, we should prefer that
      if (node.matrix.size() == 16) {
         auto floatVec = std::vector<float>{node.matrix.begin(), node.matrix.end()};
         Log::debug << "node " << node.name << " has a transform matrix" << std::endl;
         return glm::make_mat4(floatVec.data());
      } else {
         // If none of these exist, it will end up with an identity matrix
         auto translation = glm::vec3{0.f, 0.f, 0.f};
         auto rotation = glm::identity<glm::quat>();
         auto scale = glm::vec3(1.f, 1.f, 1.f);

         if (node.translation.size() == 3) {
            translation = glm::vec3(node.translation[0], node.translation[1], node.translation[2]);
         }

         if (node.rotation.size() == 4) {
            rotation =
                glm::quat(node.rotation[3], node.rotation[0], node.rotation[1], node.rotation[2]);
         }

         if (node.scale.size() == 3) {
            scale = glm::vec3(node.scale[0], node.scale[1], node.scale[2]);
         }

         auto translationMatrix = glm::translate(glm::mat4(1.f), translation);
         auto rotationMatrix = glm::mat4_cast(rotation);
         auto scaleMatrix = glm::scale(glm::mat4(1.f), scale);

         return translationMatrix * rotationMatrix * scaleMatrix;
      }
   }

   auto GeometryFactory::parseNode(const tinygltf::Model& model,
                                   const tinygltf::Node& node,
                                   std::unordered_map<int, ImageHandle>& loadedTextureIndices,
                                   TexturedGeometryHandle& handle,
                                   std::vector<GltfNode>& nodes,
                                   const int nodeIndex,
                                   const std::unordered_map<int, int>& parentMap) -> void {

      auto matrix = parseNodeTransform(node);
      {
         auto currentNodeIndex = nodeIndex;
         while (parentMap.find(currentNodeIndex) != parentMap.end()) {
            const auto& currentNode = model.nodes[currentNodeIndex];
            matrix *= parseNodeTransform(currentNode);
            currentNodeIndex = parentMap.at(currentNodeIndex);
         }
      }

      auto gltfNode =
          GltfNode{.index = nodeIndex, .number = static_cast<int>(nodes.size()), .name = node.name};
      nodes.push_back(gltfNode);

      if (node.mesh != -1) {
         const auto& mesh = model.meshes[node.mesh];
         for (const auto& primitive : mesh.primitives) {

            const auto geometryHandle = createGeometry(model, primitive, matrix);

            auto imageHandle = ImageHandle{};

            const auto materialIndex = primitive.material;
            const auto& material = model.materials[materialIndex];

            const auto& baseColorTextureIndex =
                material.pbrMetallicRoughness.baseColorTexture.index;
            if (baseColorTextureIndex != -1) {
               auto it = loadedTextureIndices.find(baseColorTextureIndex);
               if (it != loadedTextureIndices.end()) {
                  imageHandle = it->second;
               } else {
                  imageHandle = createTexture(model, baseColorTextureIndex);
                  loadedTextureIndices.insert({baseColorTextureIndex, imageHandle});
               }
            }
            handle.insert({geometryHandle, imageHandle});
         }
      }
      // Exit Criteria is node.children is empty
      for (auto& child : node.children) {
         auto& node = model.nodes[child];
         parseNode(model, node, loadedTextureIndices, handle, nodes, child, parentMap);
      }
   }

   auto GeometryFactory::buildParentMap(const tinygltf::Model& model)
       -> std::unordered_map<int, int> {
      auto parentMap = std::unordered_map<int, int>{};

      for (const auto& scene : model.scenes) {
         for (const auto& nodeIndex : scene.nodes) {
            auto stack = std::vector{nodeIndex};
            while (!stack.empty()) {
               int currentIndex = stack.back();
               stack.pop_back();

               const auto& node = model.nodes[currentIndex];
               for (const auto childIndex : node.children) {
                  parentMap[childIndex] = currentIndex;
                  stack.push_back(childIndex);
               }
            }
         }
      }

      return parentMap;
   }

   auto GeometryFactory::createGeometry(const tinygltf::Model& model,
                                        const tinygltf::Primitive& primitive,
                                        const glm::mat4& transform) -> GeometryHandle {
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
                  auto vertexPosition = glm::make_vec3(&data[i * 3]);
                  auto tempVec = transform * glm::vec4(vertexPosition, 1.f);
                  vertices[i].pos = glm::vec3(tempVec);
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
            if (attribute.first.compare("JOINTS_0") == 0) {
               Log::debug << "Contains Joints" << std::endl;
               for (size_t i = 0; i < vertexCount; i++) {
                  const auto& jointData =
                      reinterpret_cast<const uint8_t*>(&buffer.data[dataOffset]);
                  vertices[i].joint0 = glm::make_vec4(&jointData[i * 4]);
               }
            }
            if (attribute.first.compare("WEIGHTS_0") == 0) {
               Log::debug << "Contains Weights" << std::endl;
               for (size_t i = 0; i < vertexCount; i++) {
                  vertices[i].weight0 = glm::make_vec4(&data[i * 4]);
               }
            }
         }

         const auto geometryHandle = geometryKey.getKey();
         geometryDataMap.emplace(geometryHandle, GeometryData{vertices, indices});

         return geometryHandle;
      }
   }

   auto GeometryFactory::createTexture(const tinygltf::Model& model, std::size_t textureIndex)
       -> ImageHandle {
      ZoneNamedN(a, "createTexture", true);
      const auto& texture = model.textures[textureIndex];
      const auto& image = model.images[texture.source];

      const auto imageHandle = imageKey.getKey();
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