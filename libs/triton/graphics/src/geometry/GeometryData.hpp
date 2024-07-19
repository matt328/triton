#pragma once

#include "geometry/VertexStruct.hpp"

namespace tr::gfx::geo {

   class GeometryData {
    public:
      std::vector<VertexData> vertices;
      std::vector<uint32_t> indices;

      [[nodiscard]] auto vertexDataSize() const {
         return sizeof(vertices[0]) * vertices.size();
      }

      [[nodiscard]] auto indexDataSize() const {
         return sizeof(indices[0]) * indices.size();
      }
   };

   class ImageData {
    public:
      std::vector<unsigned char> data{};
      int width{};
      int height{};
      int component{};
   };

}
