#pragma once

#include "as/Vertex.hpp"
namespace tr::gfx::geo {

   class GeometryData {
    public:
      std::vector<as::Vertex> vertices;
      std::vector<uint32_t> indices;

      [[nodiscard]] auto vertexDataSize() const {
         return sizeof(vertices[0]) * vertices.size();
      }

      [[nodiscard]] auto indexDataSize() const {
         return sizeof(indices[0]) * indices.size();
      }
   };

}
