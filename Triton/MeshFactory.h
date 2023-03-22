#pragma once
#include "ImmediateContext.h"
#include "vma_raii.h"

#include "Mesh.h"
#include "Vertex.h"

#include <memory>
#include <string_view>

class MeshFactory {
 public:
   MeshFactory(const vma::raii::Allocator& allocator, const ImmediateContext& transferContext);
   ~MeshFactory() = default;

   std::unique_ptr<Mesh<Models::Vertex, uint32_t>>&& loadMeshFromGltf(
       const std::string_view& filename) const;

 private:
   const vma::raii::Allocator& allocator;
   const ImmediateContext& transferContext;
};
