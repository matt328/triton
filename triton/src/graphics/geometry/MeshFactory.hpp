#pragma once
#include "graphics/ImmediateContext.hpp"
#include "core/vma_raii.hpp"

#include "Mesh.hpp"
#include "graphics/pipeline/Vertex.hpp"

class MeshFactory {
 public:
   MeshFactory(vma::raii::Allocator* allocator, ImmediateContext* transferContext);
   ~MeshFactory() = default;
   MeshFactory(const MeshFactory&) = default;
   MeshFactory(MeshFactory&&) = delete;
   MeshFactory& operator=(const MeshFactory&) = delete;
   MeshFactory& operator=(MeshFactory&&) = delete;

   [[nodiscard]] std::unique_ptr<Mesh<Models::Vertex, uint32_t>> loadMeshFromGltf(
       const std::string_view& filename) const;

 private:
   vma::raii::Allocator* allocator;
   ImmediateContext* transferContext;
};