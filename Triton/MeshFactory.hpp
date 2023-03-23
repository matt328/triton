#pragma once
#include "ImmediateContext.hpp"
#include "vma_raii.hpp"

#include "Mesh.hpp"
#include "Vertex.hpp"

class MeshFactory {
 public:
   MeshFactory(const vma::raii::Allocator& allocator, const ImmediateContext& transferContext);
   ~MeshFactory() = default;

   std::unique_ptr<Mesh<Models::Vertex, uint32_t>> loadMeshFromGltf(
       const std::string_view& filename) const;

 private:
   const vma::raii::Allocator& allocator;
   const ImmediateContext& transferContext;
};
