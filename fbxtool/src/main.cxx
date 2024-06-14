#include "ufbx.h"
#include <glm/fwd.hpp>
#include <vector>

struct Vertex {
   glm::vec3 position;
   glm::vec3 normal;
   glm::vec2 uv;
   glm::uvec4 joints;
   glm::vec4 weights;
};

int main() {

   const auto filename = R"(C:\Users\Matt\Projects\game-assets\models\working\Walking.fbx)";

   auto loadOpts =
       ufbx_load_opts{.target_axes = ufbx_axes_right_handed_y_up, .target_unit_meters = 1.f};

   auto fbxError = ufbx_error{};

   auto scene = ufbx_load_file(filename, &loadOpts, &fbxError);

   if (!scene) {
      fprintf(stderr, "Failed to load scene: %s\n", fbxError.description.data);
      return 1;
   }

   auto& skin = scene->meshes[0]->skin_deformers[0];

   constexpr size_t MAX_WEIGHTS = 4;

   auto vertices = std::vector<Vertex>{};

   for (const auto& mesh : scene->meshes) {
      for (const auto& face : mesh->faces) {
         for (uint32_t vertex = 0; vertex < face.num_indices; vertex++) {
            auto index = face.index_begin + vertex;
            auto position = ufbx_get_vertex_vec3(&mesh->vertex_position, index);
            auto normal = ufbx_get_vertex_vec3(&mesh->vertex_normal, index);
            auto uv = ufbx_get_vertex_vec2(&mesh->vertex_uv, index);

            auto joints = std::vector<uint32_t>{};
            auto weights = std::vector<float>{};

            auto vertexIndex = mesh->vertex_indices.data[index];

            auto skinVertex = skin->vertices.data[vertexIndex];
            size_t numWeights = skinVertex.num_weights;
            if (numWeights > MAX_WEIGHTS) {
               numWeights = MAX_WEIGHTS;
            }

            auto totalWeight = 0.f;
            for (size_t i = 0; i < numWeights; ++i) {
               auto skinWeight = skin->weights.data[skinVertex.weight_begin + i];
               joints.push_back(skinWeight.cluster_index);
               weights.push_back((float)skinWeight.weight);
               totalWeight += (float(skinWeight.weight));
            }
            while (joints.size() < MAX_WEIGHTS) {
               joints.push_back(0.f);
            }

            while (weights.size() < MAX_WEIGHTS) {
               weights.push_back(0.f);
            }

            for (size_t i = 0; i < numWeights; i++) {
               weights[i] /= totalWeight;
            }

            vertices.push_back(
                Vertex{.position = glm::vec3(position.x, position.y, position.z),
                       .normal = glm::vec3(normal.x, normal.y, normal.z),
                       .uv = glm::vec2(uv.x, uv.y),
                       .joints = glm::uvec4(joints[0], joints[1], joints[2], joints[3]),
                       .weights = glm::vec4(weights[0], weights[1], weights[2], weights[3])});
         }
      }
   }

   ufbx_free_scene(scene);

   return 0;
}
