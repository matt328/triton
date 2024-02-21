#pragma once
#include "graphics/ImmediateContext.hpp"
#include "graphics/vma_raii.hpp"

#include "Mesh.hpp"
#include "Vertex.hpp"

namespace Triton::Graphics::Geometry {

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