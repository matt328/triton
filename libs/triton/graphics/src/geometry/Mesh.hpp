#pragma once

/*
   Create Mesh
   GeometryFactory produces Geometry - Vertices and Indices
   ResourceManager turns Geometry into Mesh

   - use staging buffer to transfer data into GPU Buffers - ResourceManager
   - Return an opaque MeshHandle
*/

#include "mem/Buffer.hpp"

namespace tr::gfx::geo {

   /// Aggregate struct that ties together an index buffer and a vertex buffer, and provides a
   /// convenience count of the number of indices contained within the buffer.
   struct Mesh {
      std::unique_ptr<mem::Buffer> vertexBuffer;
      std::unique_ptr<mem::Buffer> indexBuffer;
      uint32_t indicesCount;
   };

   class ImmutableMesh {
    public:
      ImmutableMesh() = delete;
      ~ImmutableMesh() = default;
      // Only allow move construction
      ImmutableMesh(const ImmutableMesh&) = delete;
      ImmutableMesh(ImmutableMesh&&) noexcept = default;
      ImmutableMesh& operator=(ImmutableMesh&&) = delete;
      ImmutableMesh& operator=(const ImmutableMesh&) = delete;

      ImmutableMesh(std::unique_ptr<mem::Buffer>&& vertexBuffer,
                    std::unique_ptr<mem::Buffer>&& indexBuffer,
                    const uint32_t indicesCount)
          : vertexBuffer(std::move(vertexBuffer)),
            indexBuffer(std::move(indexBuffer)),
            indicesCount(indicesCount) {
      }

      [[nodiscard]] const std::unique_ptr<mem::Buffer>& getVertexBuffer() const {
         return vertexBuffer;
      }

      [[nodiscard]] const std::unique_ptr<mem::Buffer>& getIndexBuffer() const {
         return indexBuffer;
      }

      [[nodiscard]] uint32_t getIndicesCount() const {
         return indicesCount;
      }

    private:
      std::unique_ptr<mem::Buffer> vertexBuffer;
      std::unique_ptr<mem::Buffer> indexBuffer;
      uint32_t indicesCount;
   };
}