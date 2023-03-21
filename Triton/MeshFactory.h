#pragma once

struct Mesh;

class MeshFactory {
 public:
   MeshFactory() = default;
   ~MeshFactory() = default;

   std::unique_ptr<Mesh> loadMeshFromGltf(const std::string_view& filename);
};
