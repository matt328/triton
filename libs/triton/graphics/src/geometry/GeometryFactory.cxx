#include "GeometryFactory.hpp"

#include "GeometryData.hpp"
#include "geometry/Vertex.hpp"
#include "HeightField.hpp"
#include "geometry/GeometryHandles.hpp"

namespace tr::gfx::geo {

   GeometryFactory::GeometryFactory() {
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
      constexpr auto NormalY = 2.f;
      int left = std::max(x - 1, 0);
      int right = std::min(x + 1, heightField.getWidth() - 1);
      int up = std::max(y - 1, 0);
      int down = std::min(y + 1, heightField.getWidth() - 1);

      float dx = heightField.valueAt(y, right) - heightField.valueAt(y, left);
      float dy = heightField.valueAt(down, x) - heightField.valueAt(up, x);

      return glm::normalize(glm::vec3(-dx, NormalY, dy));
   }

   auto GeometryFactory::loadSkinnedModel(
       [[maybe_unused]] const std::filesystem::path& modelPath,
       [[maybe_unused]] const std::filesystem::path& skeletonPath,
       [[maybe_unused]] const std::filesystem::path& animationPath) -> SkinnedGeometryData {
      auto sgd = SkinnedGeometryData{};
      // TODO: don't load gltf with engine, only trm
      //  try {

      //    const auto skeletonHandle = animationFactory.loadSkeleton(skeletonPath);
      //    const auto animationHandle = animationFactory.loadAnimation(animationPath);

      //    auto sgd = loadAnimatedGeometryFromGltf(modelPath, skeletonHandle);

      //    sgd.animationHandle = animationHandle;
      //    return sgd;
      // } catch (const std::exception& ex) {
      //    Log::error << "Error during loadGeometryFromGltf: " << ex.what() << std::endl;
      // }
      return sgd;
   }

   [[nodiscard]] auto GeometryFactory::getGeometryData(const GeometryHandle& handle)
       -> GeometryData& {
      return geometryDataMap.at(handle);
   }

   [[nodiscard]] auto GeometryFactory::getImageData(const ImageHandle& handle) -> ImageData& {
      return imageDataMap.at(handle);
   }

}