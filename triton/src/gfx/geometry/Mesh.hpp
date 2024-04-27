#pragma once
#include "../vma_raii.hpp"

/*
   Create Mesh
   GeometryFactory produces Geometry - Vertices and Indices
   ResourceManager turns Geometry into Mesh

   - use staging buffer to transfer data into GPU Buffers - ResourceManager
   - Return an opaque MeshHandle
*/

namespace tr::gfx::geo {

   /// Aggregate struct that ties together an index buffer and a vertex buffer, and provides a
   /// convenience count of the number of indices contained within the buffer.
   struct Mesh {
      std::unique_ptr<AllocatedBuffer> vertexBuffer;
      std::unique_ptr<AllocatedBuffer> indexBuffer;
      uint32_t indicesCount;
   };
}