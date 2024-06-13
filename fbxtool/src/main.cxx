#include "ufbx.h"
#include <vector>

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

   for (size_t i = 0; i < scene->nodes.count; i++) {
      ufbx_node* node = scene->nodes.data[i];
   }

   auto& skin = scene->meshes[0]->skin_deformers[0];

   constexpr size_t MAX_WEIGHTS = 4;

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

            for (size_t i = 0; i < numWeights; i++) {
               weights[i] /= totalWeight;
            }
         }
      }
   }

   ufbx_free_scene(scene);

   return 0;
}
