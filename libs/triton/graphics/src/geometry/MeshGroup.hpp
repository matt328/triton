#pragma once

#include "mem/Buffer.hpp"

namespace tr::gfx::geo {

   struct MeshDescriptor {
      uint16_t vertexOffset;
      uint16_t vertexCount;

      uint16_t indexOffset;
      uint16_t indexCount;
   };

   class MeshGroup {
    public:
      MeshGroup();
      ~MeshGroup();

    private:
      std::vector<MeshDescriptor> meshDescriptors;

      std::unique_ptr<mem::Buffer> vertexBuffer;
      std::unique_ptr<mem::Buffer> indexBuffer;
   };
}
