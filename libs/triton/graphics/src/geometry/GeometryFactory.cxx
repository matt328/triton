#include "GeometryFactory.hpp"

#include "as/Vertex.hpp"
#include "as/Model.hpp"

#include "GeometryData.hpp"
#include "HeightField.hpp"
#include "geometry/GeometryHandles.hpp"
#include <cereal/details/helpers.hpp>
#include <cereal/archives/binary.hpp>

namespace tr::gfx::geo {

   GeometryFactory::GeometryFactory() {
      const auto imageHandle = 0;
      const auto data = std::vector<unsigned char>(4, 255);
      imageDataMap.emplace(imageHandle, as::ImageData{data, 1, 1, 4});
   }

   GeometryFactory::~GeometryFactory() {
   }

   void GeometryFactory::unload(const TexturedGeometryHandle& handle) {
      for (const auto& p : handle) {
         geometryDataMap.erase(p.first);
         imageDataMap.erase(p.second);
      }
   }

   auto GeometryFactory::loadTrm(const std::filesystem::path& modelPath) -> TritonModelData {

      Log.debug("Loading TRM File: {0}", modelPath.string());

      auto tritonModel = loadTrmFile(modelPath.string());

      Log.debug("Joint Remaps Size: {0}", tritonModel.jointRemaps.size());
      for (const auto& [position, sortedIndex] : tritonModel.jointRemaps) {
         Log.debug("Joint Remap: {0}, {1}", position, sortedIndex);
      }

      Log.debug("inverseBindPoses.size(): {0}", tritonModel.inverseBindPoses.size());

      const auto imageHandle = imageKey.getKey();
      imageDataMap.emplace(imageHandle, tritonModel.imageData);

      const auto geometryHandle = geometryKey.getKey();
      geometryDataMap.emplace(geometryHandle,
                              GeometryData{tritonModel.vertices, tritonModel.indices});

      auto skinData = std::optional<cm::SkinData>{};
      if (tritonModel.skinned()) {
         skinData = cm::SkinData(tritonModel.jointRemaps, tritonModel.inverseBindPoses);
      }

      return TritonModelData{geometryHandle, imageHandle, skinData};
   }

   auto GeometryFactory::loadTrmFile(const std::string& modelPath) -> as::Model {
      try {
         ZoneNamedN(z, "Loading Model from Disk", true);
         auto is = std::ifstream(modelPath, std::ios::binary);
         if (!is) {
            throw std::runtime_error("Failed to open file: " + modelPath);
         }

         cereal::BinaryInputArchive input(is);
         auto tritonModel = as::Model{};

         input(tritonModel);
         return tritonModel;
      } catch (const std::exception& ex) {
         throw IOException(fmt::format("Error loading model: {0}", ex.what()));
      }
   }

   auto GeometryFactory::createGeometryFromHeightfield(const ct::HeightField& heightField)
       -> TexturedGeometryHandle {
      auto vertices = std::vector<as::Vertex>{};
      auto indices = std::vector<uint32_t>{};
      auto width = heightField.getWidth();

      const float scaleFactor = 25.f;

      for (int x = 0; x < width; x++) {
         for (int y = 0; y < width; y++) {
            as::Vertex vert{};
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
      imageDataMap.emplace(imageHandle, as::ImageData{data, 1, 1, 4});

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

   [[nodiscard]] auto GeometryFactory::getGeometryData(const GeometryHandle& handle)
       -> GeometryData {
      auto it = geometryDataMap.find(handle);
      if (it != geometryDataMap.end()) {
         return geometryDataMap.at(handle);
      } else {
         throw GeometryDataNotFoundException(
             fmt::format("Geometry Data with handle {0} was not found", handle));
      }
   }

   [[nodiscard]] auto GeometryFactory::getImageData(const ImageHandle& handle) -> as::ImageData& {
      auto it = imageDataMap.find(handle);
      if (it != imageDataMap.end()) {
         return imageDataMap.at(handle);
      } else {
         throw GeometryDataNotFoundException(
             fmt::format("Image Data with handle {0} was not found", handle));
      }
   }

}
