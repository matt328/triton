#include "GeometryFactory.hpp"

#include "as/Vertex.hpp"
#include "as/Model.hpp"

#include "GeometryData.hpp"
#include "geo/HeightField.hpp"
#include "cm/Handles.hpp"
#include "geo/GeometryHandles.hpp"
#include "geo/Transvoxel.hpp"

#include "cm/sdf/VoxelMetrics.hpp"

#include "GlmCereal.hpp"

// NOLINTBEGIN

namespace tr::gfx::geo {

   using cm::sdf::VoxelDebugger;

   GeometryFactory::GeometryFactory() {
      constexpr auto imageHandle = 0;
      const auto data = std::vector<unsigned char>(4, 255);
      imageDataMap.emplace(imageHandle, as::ImageData{data, 1, 1, 4});
   }

   GeometryFactory::~GeometryFactory() { // NOLINT(*-use-equals-default)
   }

   auto GeometryFactory::sdfPlane(const glm::vec3& point,
                                  [[maybe_unused]] const glm::vec3& normal,
                                  [[maybe_unused]] float distance) -> int8_t {

      int8_t result = 0;

      if (point.y > 1) {
         result = 64;
      } else if (point.y <= 1) {
         result = -64;
      }
      assert(result != 0);

      return result;
   }

   // TODO: Move this algorithm out into the application so we can create an ImGui tool
   // to step through and debug it.
   auto GeometryFactory::createTerrain() -> TexturedGeometryHandle {
      auto vertices = std::vector<as::Vertex>{};
      auto indices = std::vector<uint32_t>{};

      auto voxelData = VoxelArray{{{}}};

      for (size_t xCoord = 0; xCoord < Size; ++xCoord) {
         for (size_t yCoord = 0; yCoord < Size; ++yCoord) {
            for (size_t zCoord = 0; zCoord < Size; ++zCoord) {
               auto value = sdfPlane(glm::ivec3(xCoord, yCoord, zCoord), glm::ivec3(0, 1, 0), .5);
               assert(value != 0);
               voxelData[xCoord][yCoord][zCoord] = value;
            }
         }
      }

      // Min will always be zero for now. It's an offset into the array of chunks and we're starting
      // with just one chunk.
      auto min = glm::zero<glm::ivec3>();

      // Visit cells in the xz plane first, then repeat 'up' the y direction
      auto boxes = std::vector<GeometryHandle>{};
      for (size_t yCoord = 0; yCoord < Size - 1; ++yCoord) {
         for (size_t zCoord = 0; zCoord < Size - 1; ++zCoord) {
            for (size_t xCoord = 0; xCoord < Size - 1; ++xCoord) {
               auto position = glm::ivec3(xCoord, yCoord, zCoord);
               polygonizeCell(min, position, vertices, indices, voxelData, boxes);
            }
         }
      }

      for (const auto& index : indices) {
         Log.debug("Index: {0}", index);
      }

      const auto imageHandle = imageKey.getKey();
      const auto data = std::vector<unsigned char>(4, 255);
      imageDataMap.emplace(imageHandle, as::ImageData{data, 1, 1, 4});

      const auto key = geometryKey.getKey();
      const auto geometryHandle = GeometryHandle{key, cm::Topology::Triangles};
      geometryDataMap.emplace(geometryHandle, GeometryData{vertices, indices});

      auto val = TexturedGeometryHandle{{geometryHandle, imageHandle}};
      for (const auto& box : boxes) {
         val[box] = imageHandle;
      }
      return val;
   }

   void GeometryFactory::polygonizeCell(glm::ivec3& offsetPosition,
                                        glm::ivec3& cellPosition,
                                        std::vector<as::Vertex>& vertices,
                                        std::vector<uint32_t>& indices,
                                        const VoxelArray& voxelData,
                                        std::vector<GeometryHandle>& boxes) {

      /// The position of the current cube (cell) in world chunk space.
      auto currentOffsetPosition = offsetPosition + cellPosition;

      /// Encodes the direction (+/-) of x,z,y in the first 3 bits of this byte. If the direction
      /// bit is 1, that means a cell exists in that direction, and we can reuse its vertices.
      /// 1110 -> xzy, 1010 -> xy only
      /// TODO: The impl has the order x, z, y here, but the paper mentions x, y, z. I think the
      /// paper uses a different coordinate system with z vertical.
      int8_t directionMask = (cellPosition.x > 0 ? 1 : 0) | ((cellPosition.z > 0 ? 1 : 0) << 1) |
                             ((cellPosition.y > 0 ? 1 : 0) << 2);

      /// The current cube's corner values from the sdf, as described by fig 3.7
      std::array<int8_t, 8> corner{};
      for (int8_t currentCorner = 0; currentCorner < 8; ++currentCorner) {
         const auto voxelPosition = currentOffsetPosition + CornerIndex[currentCorner];
         auto voxelValue = voxelData[voxelPosition.x][voxelPosition.y][voxelPosition.z];
         assert(voxelValue != 0);
         corner[currentCorner] = voxelValue;
      }

      // TODO: given the sdf values at the corners, the case code is not being generated correctly

      /// The corner value in the SDF being non-negative means outside, negative means inside
      /// This code packs only the corner values' sign bits into a single 8 bit value which is how
      /// Lengyel's CellClass and CellData tables are indexed.
      uint8_t caseCode = ((corner[0] >> 7) & 0x01) << 7 | // Corner 0
                         ((corner[1] >> 7) & 0x01) << 6 | // Corner 1
                         ((corner[2] >> 7) & 0x01) << 5 | // Corner 2
                         ((corner[3] >> 7) & 0x01) << 4 | // Corner 3
                         ((corner[4] >> 7) & 0x01) << 3 | // Corner 4
                         ((corner[5] >> 7) & 0x01) << 2 | // Corner 5
                         ((corner[6] >> 7) & 0x01) << 1 | // Corner 6
                         ((corner[7] >> 7) & 0x01);       // Corner 7

      // can bail on the whole cell right here with this check
      // 0 means the whole cell is either above or below, in either case, no verts are generated

      auto validCell = (caseCode ^ ((corner[7] >> 7) & 0xFF)) != 0;

      if (validCell) {
         const auto maxPosition = cellPosition + glm::ivec3(1, 1, 1);
         boxes.push_back(generateAABB(cellPosition, maxPosition));
         auto equivalenceClassIndex = regularCellClass[caseCode];
         auto equivalenceClass = regularCellData[equivalenceClassIndex];

         const auto vertexCount = equivalenceClass.getVertexCount();
         const auto triangleCount = equivalenceClass.getTriangleCount();
         const auto vertexSequence = equivalenceClass.getVertexIndex();
         auto mappedIndices = std::vector<uint16_t>{};

         const auto vertexLocations = regularVertexData[caseCode];

         auto cellVertices = std::vector<as::Vertex>{};

         for (uint8_t vli = 0; vli < vertexCount; ++vli) {
            auto vertexLocation = vertexLocations[vli];
            /// Edge information is encoded in the high nibble of regularVertexData
            uint8_t edge = vertexLocation >> 8;
            /// Reuse information from the low nibble, this is the index of the vertex in the
            /// preceeding cell (cube) to use
            uint8_t reuseIndex = edge & 0xF;

            // Directions to the preceeding cell (cube) are in the high nibble.
            // bit value 1 = -x, bit value 2 = -y bit value 4 = -z bit value 8 means create new
            // vertex
            uint8_t dirPrev = edge >> 4;

            uint8_t cellCornerIndex1 = vertexLocation & 0x0F;
            uint8_t cellCornerIndex0 = (vertexLocation >> 4) & 0x0F;

            int8_t distance0 = corner[cellCornerIndex0];
            int8_t distance1 = corner[cellCornerIndex1];

            // Calculate distance from distance1 that the sign change occurs, ie the surface exists,
            // in the range of 0-256
            int32_t t = (distance1 << 8) / (distance1 - distance0);
            int32_t u = 0x0100 - t; // compliment of t

            // scale each value into [0,1]
            [[maybe_unused]] float t0 = t / 256.F;
            [[maybe_unused]] float t1 = u / 256.F;

            int index = -1;
            // If cellCornderIndex1 is 7, that means this cell does not own the vertex. And if the
            // dirPrev (which comes from the tables) & this edge's direction mask is itself, that
            // means the vertex in question could have already been generated, so we have to check.
            if (cellCornerIndex1 != 7 && (dirPrev & directionMask) == dirPrev) {
               const auto reuseCell = cache.getReusedIndex(cellPosition, dirPrev);
               index = reuseCell.vertices[reuseIndex];
            }

            if (index == -1) {
               // The cube in dirPrev did not generate a vertex on the edge we are needing one
               // so generate one.
               index = generateVertex(vertices,
                                      cellVertices,
                                      currentOffsetPosition,
                                      cellPosition,
                                      t0,
                                      cellCornerIndex0,
                                      cellCornerIndex1,
                                      distance0,
                                      distance1);
            }

            if ((dirPrev & 8) != 0) {
               // dirPrev having bit 8 set means create a new vertex
               uint16_t lastAddedVertexIndex = 0;
               cache.setReusableIndex(cellPosition, reuseIndex, lastAddedVertexIndex);
            }
            mappedIndices.push_back(index);
         }
         VoxelDebugger::getInstance().addActiveCube(cellPosition,
                                                    caseCode,
                                                    equivalenceClassIndex,
                                                    cellVertices);
         for (uint32_t t = 0; t < triangleCount; t++) {
            for (int i = 0; i < 3; ++i) {
               indices.push_back(mappedIndices[vertexSequence[t * 3 + i]]);
            }
         }
      }
   }

   auto GeometryFactory::generateVertex(std::vector<as::Vertex>& vertices,
                                        std::vector<as::Vertex>& cellVertices,
                                        [[maybe_unused]] glm::ivec3& offsetPosition,
                                        [[maybe_unused]] glm::ivec3& cellPosition,
                                        float t,
                                        uint8_t corner0,
                                        uint8_t corner1,
                                        [[maybe_unused]] int8_t distance0,
                                        [[maybe_unused]] int8_t distance1) -> int {
      auto iP0 = (offsetPosition + CornerIndex[corner0]);
      auto P0 = glm::vec3(iP0);
      auto iP1 = (offsetPosition + CornerIndex[corner1]);
      auto P1 = glm::vec3(iP1);

      auto result = glm::mix(P0, P1, t);
      auto vertex = as::Vertex{};
      vertex.pos = result;
      auto size = vertices.size();
      Log.debug("Vertex: {0}", vertex);
      vertices.push_back(vertex);

      cellVertices.push_back(vertex);

      return size;
   }

   void GeometryFactory::unload(const TexturedGeometryHandle& handle) {
      for (const auto& [geometry, texture] : handle) {
         geometryDataMap.erase(geometry);
         imageDataMap.erase(texture);
      }
   }

   /// Loads a TRM model.
   /// @throws IOException if there's an issue opening or reading the file.
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

      const auto key = geometryKey.getKey();
      const auto geometryHandle = GeometryHandle{key, cm::Topology::Triangles};
      geometryDataMap.emplace(geometryHandle,
                              GeometryData{tritonModel.vertices, tritonModel.indices});

      auto skinData = std::optional<cm::SkinData>{};
      if (tritonModel.skinned()) {
         skinData = cm::SkinData(tritonModel.jointRemaps, tritonModel.inverseBindPoses);
      }

      return TritonModelData{geometryHandle, imageHandle, skinData};
   }

   auto GeometryFactory::loadTrmFile(const std::filesystem::path& modelPath) -> as::Model {
      try {
         ZoneNamedN(z, "Loading Model from Disk", true);
         if (modelPath.extension() == ".trm") {
            auto is = std::ifstream(modelPath, std::ios::binary);
            if (!is) {
               throw IOException("Failed to open file: " + modelPath.string());
            }

            try {
               cereal::PortableBinaryInputArchive input(is);
               auto tritonModel = as::Model{};
               input(tritonModel);
               return tritonModel;
            } catch (const std::exception& ex) {
               throw IOException("Error reading: " + modelPath.string() + ": ", ex);
            }
         } else if (modelPath.extension() == ".json") {
            auto is = std::ifstream(modelPath);
            if (!is) {
               throw IOException("Failed to open file: " + modelPath.string());
            }
            try {
               cereal::JSONInputArchive input(is);
               auto tritonModel = as::Model{};
               input(tritonModel);
               return tritonModel;
            } catch (const std::exception& ex) {
               throw IOException("Error reading: " + modelPath.string() + ": ", ex);
            }
         }
      } catch (BaseException& ex) {
         ex << "GeometryFactory::loadTrmFile(): ";
         throw;
      }
      Log.warn("loadTrmFile returning empty model");
      return {};
   }

   auto GeometryFactory::createGeometryFromHeightfield(const ct::HeightField& heightField)
       -> TexturedGeometryHandle {
      auto vertices = std::vector<as::Vertex>{};
      auto indices = std::vector<uint32_t>{};
      auto width = heightField.getWidth();

      for (int x = 0; x < width; x++) {
         for (int y = 0; y < width; y++) {
            constexpr float scaleFactor = 25.f;
            as::Vertex vert{};
            vert.pos = glm::vec4(static_cast<float>(x) * scaleFactor,
                                 heightField.valueAt(x, y) * scaleFactor,
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
         const auto faceNormal = normalize(cross(edge1, edge2));
         for (size_t j = 0; j < 3; ++j) {
            vertices[indices[i + j]].normal += faceNormal;
         }
      }

      // Normalize normals
      for (auto& [pos, normal, uv, color, joint0, weight0, tangent] : vertices) {
         normal = normalize(normal);
      }

      const auto imageHandle = imageKey.getKey();
      const auto data = std::vector<unsigned char>(4, 255);
      imageDataMap.emplace(imageHandle, as::ImageData{data, 1, 1, 4});

      const auto key = geometryKey.getKey();
      const auto geometryHandle = GeometryHandle{key, cm::Topology::Triangles};
      geometryDataMap.emplace(geometryHandle, GeometryData{vertices, indices});

      return {{geometryHandle, imageHandle}};
   }

   auto GeometryFactory::generateNormal(const int x,
                                        const int y,
                                        const ct::HeightField& heightField) -> glm::vec3 {
      constexpr auto NormalY = 2.f;
      const int left = std::max(x - 1, 0);
      const int right = std::min(x + 1, heightField.getWidth() - 1);
      const int up = std::max(y - 1, 0);
      const int down = std::min(y + 1, heightField.getWidth() - 1);

      const float dx = heightField.valueAt(y, right) - heightField.valueAt(y, left);
      const float dy = heightField.valueAt(down, x) - heightField.valueAt(up, x);

      return normalize(glm::vec3(-dx, NormalY, dy));
   }

   auto GeometryFactory::generateAABB(const glm::vec3& min, const glm::vec3& max)
       -> GeometryHandle {

      auto vertices = std::vector<as::Vertex>{{
          {{min.x, min.y, min.z}}, // 0: Bottom-left-back
          {{max.x, min.y, min.z}}, // 1: Bottom-right-back
          {{max.x, max.y, min.z}}, // 2: Top-right-back
          {{min.x, max.y, min.z}}, // 3: Top-left-back
          {{min.x, min.y, max.z}}, // 4: Bottom-left-front
          {{max.x, min.y, max.z}}, // 5: Bottom-right-front
          {{max.x, max.y, max.z}}, // 6: Top-right-front
          {{min.x, max.y, max.z}}  // 7: Top-left-front
      }};

      std::vector<uint32_t> indices = {0, 1, 1, 2, 2, 3, 3, 0, 4, 5, 5, 6,
                                       6, 7, 7, 4, 0, 4, 1, 5, 2, 6, 3, 7};

      const auto key = geometryKey.getKey();
      auto geometryHandle = GeometryHandle{key, cm::Topology::LineList};
      geometryDataMap.emplace(geometryHandle, GeometryData{vertices, indices});
      return geometryHandle;
   }

   [[nodiscard]] auto GeometryFactory::getGeometryData(const GeometryHandle& handle)
       -> GeometryData {
      if (const auto it = geometryDataMap.find(handle); it != geometryDataMap.end()) {
         return geometryDataMap.at(handle);
      }
      throw GeometryDataNotFoundException(
          fmt::format("Geometry Data with handle {0} was not found", handle.handle));
   }

   [[nodiscard]] auto GeometryFactory::getImageData(const ImageHandle& handle) -> as::ImageData& {
      if (const auto it = imageDataMap.find(handle); it != imageDataMap.end()) {
         return imageDataMap.at(handle);
      }
      throw GeometryDataNotFoundException(
          fmt::format("Image Data with handle {0} was not found", handle));
   }
}
// NOLINTEND