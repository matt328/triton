#pragma once
#include "gfx/ImmediateContext.hpp"
#include "gfx/vma_raii.hpp"

#include "Mesh.hpp"
#include "Vertex.hpp"

namespace tr::gfx {
   class ImmediateContext;
}

namespace tr::gfx::Geometry {

   class MeshFactory {
    public:
      MeshFactory(Allocator* allocator, ImmediateContext* transferContext);
      ~MeshFactory() = default;
      MeshFactory(const MeshFactory&) = default;
      MeshFactory(MeshFactory&&) = delete;
      MeshFactory& operator=(const MeshFactory&) = delete;
      MeshFactory& operator=(MeshFactory&&) = delete;

      [[nodiscard]] std::unique_ptr<Mesh<Vertex, uint32_t>> loadMeshFromGltf(
          const std::string_view& filename) const;

    private:
      Allocator* allocator;
      ImmediateContext* transferContext;
   };
}