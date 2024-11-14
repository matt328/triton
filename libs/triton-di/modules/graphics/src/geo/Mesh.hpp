#pragma once

#include "mem/Buffer.hpp"

namespace tr::gfx::geo {

   class ImmutableMesh {
    public:
      ImmutableMesh() = delete;
      ~ImmutableMesh() = default;

      ImmutableMesh(const ImmutableMesh&) = delete;
      ImmutableMesh(ImmutableMesh&&) noexcept = default;
      auto operator=(ImmutableMesh&&) -> ImmutableMesh& = delete;
      auto operator=(const ImmutableMesh&) -> ImmutableMesh& = delete;

      ImmutableMesh(std::unique_ptr<mem::Buffer>&& vertexBuffer,
                    std::unique_ptr<mem::Buffer>&& indexBuffer,
                    const uint32_t indicesCount)
          : vertexBuffer(std::move(vertexBuffer)),
            indexBuffer(std::move(indexBuffer)),
            indicesCount(indicesCount) {
      }

      [[nodiscard]] auto getVertexBuffer() const -> const std::unique_ptr<mem::Buffer>& {
         return vertexBuffer;
      }

      [[nodiscard]] auto getIndexBuffer() const -> const std::unique_ptr<mem::Buffer>& {
         return indexBuffer;
      }

      [[nodiscard]] auto getIndicesCount() const -> uint32_t {
         return indicesCount;
      }

    private:
      std::unique_ptr<mem::Buffer> vertexBuffer;
      std::unique_ptr<mem::Buffer> indexBuffer;
      uint32_t indicesCount;
   };
}
